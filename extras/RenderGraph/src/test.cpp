
#include <LabRenderGraph/LabRenderGraph.h>
#include <stdio.h>
#include <vector>
#include <sys/stat.h>

int main(int argc, char** argv)
{
    char* filename = "C:/Projects/meshula/src/LabRender/assets/pipelines/deferred-offscreen.labfx";
    struct stat st;
    if (stat(filename, &st) != 0) 
    {
        return 0;
    }
    size_t sz = st.st_size;

    FILE* f = fopen(filename, "rb");
    std::vector<char> buff(sz);
    fread(&buff[0], 1, sz, f);
    fclose(f);
    
    void* fx = parse_labfx(&buff[0], sz);
    free_labfx(fx);
    return 1;
}
