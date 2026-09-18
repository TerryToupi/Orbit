#include <render/gpu_transfer_queue.h>
#include <cassert>

bool begin_gpu_uploads(GPUTransferQueue& queue, SDL_GPUDevice* device, size_t bytes, uint32_t upload_count)
{
    assert(!queue.mapped && !queue.count && !queue.used);
    queue.submitted = false;
    if (!bytes)
        return true;
    if (bytes > queue.capacity) {
        SDL_GPUTransferBufferCreateInfo info = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = uint32_t((bytes + 65535) / 65536 * 65536)};
        SDL_GPUTransferBuffer* staging = SDL_CreateGPUTransferBuffer(device, &info);
        if (!staging)
            return false;
        if (queue.staging) SDL_ReleaseGPUTransferBuffer(device, queue.staging);
        queue.staging = staging;
        queue.capacity = info.size;
    }
    if (upload_count > queue.upload_capacity) {
        queue.upload_capacity = (upload_count + 127) / 128 * 128;
        queue.uploads = static_cast<GPUBufferUpload*>(SDL_realloc(queue.uploads, sizeof(GPUBufferUpload) * queue.upload_capacity));
    }
    queue.mapped = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, queue.staging, true));
    return queue.mapped != nullptr;
}

void destroy_gpu_transfer_queue(GPUTransferQueue& queue, SDL_GPUDevice* device)
{
    reset_gpu_transfers(queue, device);
    if (queue.staging) SDL_ReleaseGPUTransferBuffer(device, queue.staging);
    SDL_free(queue.uploads);
    queue = {};
}

uint8_t* queue_gpu_upload(GPUTransferQueue& queue, SDL_GPUBuffer* buffer, size_t bytes)
{
    assert(queue.mapped && queue.count < queue.upload_capacity);
    size_t offset = (queue.used + 3) & ~size_t(3);
    assert(offset + bytes <= queue.capacity);
    queue.uploads[queue.count++] = {.buffer = buffer, .offset = uint32_t(offset), .size = uint32_t(bytes)};
    queue.used = offset + bytes;
    return queue.mapped + offset;
}

void encode_gpu_transfers(GPUTransferQueue& queue, SDL_GPUDevice* device, SDL_GPUCommandBuffer* commands)
{
    if (queue.mapped) {
        SDL_UnmapGPUTransferBuffer(device, queue.staging);
        queue.mapped = nullptr;
    }
    if (!queue.count)
        return;
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    for (uint32_t i = 0; i < queue.count; ++i) {
        const GPUBufferUpload& upload = queue.uploads[i];
        SDL_GPUTransferBufferLocation from = {.transfer_buffer = queue.staging, .offset = upload.offset};
        SDL_GPUBufferRegion to = {.buffer = upload.buffer, .size = upload.size};
        SDL_UploadToGPUBuffer(copy, &from, &to, false);
    }
    SDL_EndGPUCopyPass(copy);
}

void reset_gpu_transfers(GPUTransferQueue& queue, SDL_GPUDevice* device)
{
    if (queue.mapped) SDL_UnmapGPUTransferBuffer(device, queue.staging);
    queue.mapped = nullptr;
    queue.count = 0;
    queue.used = 0;
    queue.submitted = false;
}
