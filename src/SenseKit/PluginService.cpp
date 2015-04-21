﻿#include "PluginService.h"
#include "Stream.h"
#include "SenseKitContext.h"
#include "PluginServiceDelegate.h"
#include <SenseKit/Plugins/PluginServiceProxyBase.h>
#include <SenseKit/sensekit_types.h>
#include "CreatePluginProxy.h"
#include "ParameterBin.h"
#include "Logging.h"
#include <cstdio>
#include <memory>

namespace sensekit
{
    PluginServiceProxyBase* PluginService::create_proxy()
    {
        return create_plugin_proxy(this);
    }

    sensekit_status_t PluginService::create_stream_set(sensekit_streamset_t& streamSet)
    {
        //TODO: normally would create a new streamset
        StreamSet* actualStreamSet = &m_context.get_rootSet();
        streamSet = actualStreamSet->get_handle();

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::destroy_stream_set(sensekit_streamset_t& streamSet)
    {
        StreamSet* actualStreamSet = StreamSet::get_ptr(streamSet);

        //TODO: if we were not hard coding the rootset in create_stream_set...
        //if streamset has direct child streams, return error
        //if streamset has child streamsets, reparent them to this streamset's parent (or null parent)
        //then delete the streamset

        streamSet = nullptr;

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::register_stream_added_callback(stream_added_callback_t callback,
                                                                    void* clientTag,
                                                                    CallbackId& callbackId)
    {
        auto thunk = [clientTag, callback](sensekit_streamset_t ss,
                                           sensekit_stream_t s,
                                           sensekit_stream_desc_t d)
            {
                callback(clientTag, ss, s, d);
            };

        callbackId = m_streamAddedSignal += thunk;

        m_context.raise_existing_streams_added(callback, clientTag);

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::register_stream_removing_callback(stream_removing_callback_t callback,
                                                                       void* clientTag,
                                                                       CallbackId& callbackId)
    {
        auto thunk = [clientTag, callback](sensekit_streamset_t ss,
                                           sensekit_stream_t s,
                                           sensekit_stream_desc_t d)
            {
                callback(clientTag, ss, s, d);
            };

        callbackId = m_streamRemovingSignal += thunk;

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::unregister_stream_added_callback(CallbackId callbackId)
    {
        m_streamAddedSignal -= callbackId;

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::unregister_stream_removing_callback(CallbackId callbackId)
    {
        m_streamRemovingSignal -= callbackId;

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::create_stream(sensekit_streamset_t setHandle,
                                                   sensekit_stream_desc_t desc,
                                                   stream_callbacks_t pluginCallbacks,
                                                   sensekit_stream_t& handle)
    {
        m_logger.info("registering stream");

        // TODO add to specific stream set
        Stream* stream = m_context.get_rootSet().create_stream(desc, pluginCallbacks);
        handle = stream->get_handle();

        m_streamAddedSignal.raise(setHandle, handle, desc);

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::destroy_stream(sensekit_stream_t& streamHandle)
    {
        if (streamHandle == nullptr)
            return SENSEKIT_STATUS_INVALID_PARAMETER;

        m_logger.info("unregistered stream.");
        //TODO refactor this mess

        StreamSet* set = &m_context.get_rootSet();
        sensekit_streamset_t setHandle = set->get_handle();

        Stream* stream = Stream::get_ptr(streamHandle);
        const sensekit_stream_desc_t& desc = stream->get_description();

        m_streamRemovingSignal.raise(setHandle, streamHandle, desc);

        set->destroy_stream(stream);

        streamHandle = nullptr;

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::create_stream_bin(sensekit_stream_t streamHandle,
                                                       size_t lengthInBytes,
                                                       sensekit_bin_t& binHandle,
                                                       sensekit_frame_t*& binBuffer)
    {
        Stream* actualStream = Stream::get_ptr(streamHandle);
        StreamBin* bin = actualStream->create_bin(lengthInBytes);

        binHandle = bin->get_handle();
        binBuffer = bin->get_backBuffer();

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::destroy_stream_bin(sensekit_stream_t streamHandle,
                                                        sensekit_bin_t& binHandle,
                                                        sensekit_frame_t*& binBuffer)
    {
        Stream* actualStream = Stream::get_ptr(streamHandle);
        StreamBin* bin = StreamBin::get_ptr(binHandle);
        actualStream->destroy_bin(bin);

        binHandle = nullptr;
        binBuffer = nullptr;

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::bin_has_connections(sensekit_bin_t binHandle, bool& hasConnections)
    {
        StreamBin* bin = StreamBin::get_ptr(binHandle);
        hasConnections = bin->has_clients_connected();

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::cycle_bin_buffers(sensekit_bin_t binHandle,
                                                       sensekit_frame_t*& binBuffer)
    {
        assert(binHandle != nullptr);

        StreamBin* bin = StreamBin::get_ptr(binHandle);
        binBuffer = bin->cycle_buffers();

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::link_connection_to_bin(sensekit_streamconnection_t connection,
                                                            sensekit_bin_t binHandle)
    {
        StreamConnection* underlyingConnection = StreamConnection::get_ptr(connection);
        StreamBin* bin = StreamBin::get_ptr(binHandle);

        underlyingConnection->set_bin(bin);

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::get_parameter_bin(size_t byteSize,
                                                       sensekit_parameter_bin_t& binHandle,
                                                       sensekit_parameter_data_t& parameterData)
    {
        //TODO pooling
        ParameterBin* parameterBin = new ParameterBin(byteSize);

        binHandle = parameterBin->get_handle();
        parameterData = parameterBin->data();

        return SENSEKIT_STATUS_SUCCESS;
    }

    sensekit_status_t PluginService::log(sensekit_log_severity_t logLevel,
                                         const char* format,
                                         va_list args)
    {
        m_logger.log_vargs(logLevel, format, args);
        return SENSEKIT_STATUS_SUCCESS;
    }
}
