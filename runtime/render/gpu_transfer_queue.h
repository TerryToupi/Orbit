#ifndef ORBIT_GPU_TRANSFER_QUEUE_H
#define ORBIT_GPU_TRANSFER_QUEUE_H

#include <SDL3/SDL_gpu.h>

struct GPUBufferUpload
{
    SDL_GPUBuffer* buffer = nullptr;
    uint32_t offset = 0;
    uint32_t size = 0;
};

// Main-thread logical upload batch. No command buffer or backend queue ownership.
struct GPUTransferQueue
{
    SDL_GPUTransferBuffer* staging = nullptr;
    uint8_t* mapped = nullptr;
    GPUBufferUpload* uploads = nullptr;
    size_t capacity = 0;
    uint32_t upload_capacity = 0;
    uint32_t count = 0;
    size_t used = 0;
    bool submitted = false;
};

// Reserve the complete batch before appending. bytes includes four-byte padding between uploads.
bool begin_gpu_uploads(GPUTransferQueue& queue, SDL_GPUDevice* device, size_t bytes, uint32_t upload_count);
void destroy_gpu_transfer_queue(GPUTransferQueue& queue, SDL_GPUDevice* device);
uint8_t* queue_gpu_upload(GPUTransferQueue& queue, SDL_GPUBuffer* buffer, size_t bytes);
// Unmaps once and records one copy pass. Caller submits the transfer command buffer before rendering.
void encode_gpu_transfers(GPUTransferQueue& queue, SDL_GPUDevice* device, SDL_GPUCommandBuffer* commands);
// After submission/cancellation; also abandons a mapped batch if command acquisition failed.
void reset_gpu_transfers(GPUTransferQueue& queue, SDL_GPUDevice* device);

#endif
