#include <malloc.h>
#include <stdio.h>
#include <zlib.h>

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
        printf("Usage: zlibber [file list]");
        return -1;
    }

    for (int i = 1; i < argc; i++)
    {
        size_t FileSize = GetFileSize(argv[i]);
        if (FileSize == 0)
        {
            continue;
        }
        printf("File size: %X\n", FileSize);

        FILE *Target = fopen(argv[i], "rb");
        if (!Target)
        {
            continue;
        }

        Bytef *FileBuffer = malloc(FileSize);
        Bytef *CompressionBuffer = malloc(FileSize);
        size_t CompressionBufferSize = FileSize;
        if (fread(FileBuffer, 1, FileSize, Target) != FileSize)
        {
            goto Cleanup;
        }
        fclose(Target);
        printf("File read to RAM.\n");

        int ZError = compress2(CompressionBuffer, (uLongf *)&CompressionBufferSize, FileBuffer, FileSize, 9);
        if (ZError != Z_OK)
        {
            goto Cleanup;
        }
        printf("Compressed\n");

        Target = fopen(argv[i], "wb");
        if (!Target)
        {
            goto Cleanup;
        }
        printf("Reopened for writing.\n");

        fwrite(&FileSize, 1, sizeof(uLongf), Target);
        fwrite(&CompressionBufferSize, 1, sizeof(uLongf), Target);
        fwrite(CompressionBuffer, 1, CompressionBufferSize, Target);
        printf("Written.\n");

    Cleanup:
        free(FileBuffer);
        free(CompressionBuffer);
        fclose(Target);
    }
}
