
#include "vegascene.h"


int main(int argc, char *argv[])
{
    std::string specFilePath("/opt/shared/work/source_code/vega/vega/examples/spec/bar.json");
    std::string outFilePath("");//("/opt/shared/work/source_code/vega/out/bar-sg.json");
    vega_scene(specFilePath, outFilePath);

    return EXIT_SUCCESS;
}
