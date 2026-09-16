#ifndef ORBIT_FILE_STREAM_H
#define ORBIT_FILE_STREAM_H

#include <command_stream.h>
#include <content.h>

#include <SDL3/SDL_filesystem.h>

struct ArtifactCommand;
struct ArtifactEvent;
struct ArtifactView;
using ArtifactCommandBuffer = CommandBuffer<ArtifactCommand, ArtifactEvent>;

// Exact path spelling is the identity; callers use stable absolute paths (no cwd changes while servers run).
ContentHash file_identity(const char* path);
// Main thread records into the artifact stream. Paths are copied; results arrive as ArtifactKind::File events. Returns 0 when the buffer is full.
uint64_t file_load(ArtifactCommandBuffer& buffer, const char* path);
// File producer stages, executed by the owning artifact lane. No queue or scheduler of their own.
ArtifactView file_probe(const char* path, uint64_t size_limit, SDL_PathInfo& info, char (&error)[160]);
ArtifactView file_read(ContentCache& content, const char* path, const SDL_PathInfo& before, char (&error)[160]);

#endif
