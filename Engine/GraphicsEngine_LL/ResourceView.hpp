#pragma once

#include "HighLevelDescHeap.hpp"

#include <utility>

namespace inl {
namespace gxeng {


class VertexBuffer;
class Texture2D;
class LinearBuffer;
class ConstBuffer;

template <typename ResourceT>
class ResourceViewBase {
public:
	ResourceViewBase(const std::shared_ptr<ResourceT>& resource, DescriptorReference&& descRef) :
		m_resource(resource),
		m_descRef(std::move(descRef))
	{}

	const std::shared_ptr<ResourceT>& GetResource() {
		return m_resource;
	}

	gxapi::DescriptorHandle GetHandle() {
		return m_descRef.Get();
	}

protected:
	std::shared_ptr<ResourceT> m_resource;
	DescriptorReference m_descRef;
};


class VertexBufferView {
public:
	VertexBufferView(const std::shared_ptr<VertexBuffer>& resource, uint32_t stride, uint32_t size);

	const std::shared_ptr<VertexBuffer>& GetResource();

protected:
	std::shared_ptr<VertexBuffer> m_resource;
	uint32_t m_stride;
	uint32_t m_size;
};


class ConstBufferView : public ResourceViewBase<ConstBuffer> {
public:
	ConstBufferView(const std::shared_ptr<ConstBuffer>& resource, DescriptorReference&& descRef);
};


class RenderTargetView : public ResourceViewBase<Texture2D> {
public:
	RenderTargetView(const std::shared_ptr<Texture2D>& resource, DescriptorReference&& descRef, gxapi::RenderTargetViewDesc desc);
	
	gxapi::RenderTargetViewDesc GetDescription() const;

protected:
	gxapi::RenderTargetViewDesc m_desc;
};


class DepthStencilView : public ResourceViewBase<Texture2D> {
public:
	DepthStencilView(const std::shared_ptr<Texture2D>& resource, DescriptorReference&& descRef, gxapi::DepthStencilViewDesc desc);

	gxapi::DepthStencilViewDesc GetDescription() const;

protected:
	gxapi::DepthStencilViewDesc m_desc;
};


template <typename ResourceT>
class ShaderResourceView : public ResourceViewBase<ResourceT> {
public:
	ShaderResourceView(const std::shared_ptr<ResourceT>& resource, DescriptorReference&& descRef, gxapi::eFormat format) :
		ResourceViewBase(resource, std::move(descRef)),
		m_format(format)
	{}

protected:
	gxapi::eFormat m_format;
};


class BufferSRV : public ShaderResourceView<LinearBuffer> {
public:
	BufferSRV(const std::shared_ptr<LinearBuffer>& resource, DescriptorReference&& desc, gxapi::eFormat format, gxapi::SrvBuffer srvDesc);

	const gxapi::SrvBuffer& GetDescription() const;

protected:
	gxapi::SrvBuffer m_srvDesc;
};


} // namespace gxeng
} // namespace inl
