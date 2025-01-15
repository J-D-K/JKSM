#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <zstd.h>

typedef unsigned char byte;

static size_t GetFileSize(const char *FilePath)
{
    FILE *GetSize = fopen(FilePath, "rb");
    if (!GetSize)
    {
        return 0;
    }

    fseek(GetSize, 0, SEEK_END);
    size_t FileSize = ftell(GetSize);
    fclose(GetSize);

    return FileSize;
}

int main(int argc, const char *argv[])
{
    if (argc < 1)
    {
        printf("Usage: fontcompressor [file list]");
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        // Just print something.
        printf("Processing %s... ", argv[i]);

        // Get starting, uncompressed size.
        uint32_t FileSize = GetFileSize(argv[i]);

        // Don't bother if file is empty.
        if (FileSize == 0)
        {
            printf("\nFile is empty. Skipping.\n");
            continue;
        }

        // Open file for reading.
        FILE *Target = fopen(argv[i], "rb");
        if (!Target)
        {
            printf("\nError opening file \"%s\". Skipping.\n", argv[i]);
            continue;
        }

        // Allocate buffers for reading and compressing.
        byte *ReadBuffer = malloc(FileSize);
        byte *CompressBuffer = malloc(FileSize);
        if (!ReadBuffer || !CompressBuffer)
        {
            // Jump to cleanup and abort mission.
            printf("\nError allocating buffers.\n");
            goto Cleanup;
        }

        // Try to read entire file into RAM at once.
        if (fread(ReadBuffer, 1, FileSize, Target) != FileSize)
        {
            printf("\nError reading file to RAM.\n");
            goto Cleanup;
        }

        // Close the file.
        fclose(Target);

        // This is the simple way to compress things with ZSTD.
        // Using uint32_t instead of size_t so everything is consistent with 3DS.
        uint32_t CompressedSize = ZSTD_compress(CompressBuffer, FileSize, ReadBuffer, FileSize, 22);
        if (ZSTD_isError(CompressedSize))
        {
            printf("\nError compressing font.\n");
            goto Cleanup;
        }

        // Reopen file for writing.
        Target = fopen(argv[i], "wb");
        if (!Target)
        {
            printf("\nError opening file for writing.\n");
            goto Cleanup;
        }

        // Write the uncompressed size
        fwrite(&FileSize, sizeof(uint32_t), 1, Target);
        fwrite(&CompressedSize, sizeof(uint32_t), 1, Target);
        // Write the compressed data
        fwrite(CompressBuffer, 1, CompressedSize, Target);

        // This should only get printed if everything succeeded.
        printf("Done!\n");

    Cleanup:
        if (Target)
        {
            fclose(Target);
        }

        if (ReadBuffer)
        {
            free(ReadBuffer);
        }

        if (CompressBuffer)
        {
            free(CompressBuffer);
        }
    }
}
