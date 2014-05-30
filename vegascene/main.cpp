
#include "vegascene.h"

#include <ostream>


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage:\n"
                  << "  vegascene vega_json_file [output_json_file]\n"
                  << "  If output_json_file is not provided, writes to stdout.\n\n";
        return EXIT_FAILURE;
    }
    else
    {
        std::string specFilePath(argv[1]);
        std::string outFilePath("");
        if (argc > 2)
        {
            outFilePath = String(argv[2]);
        }
        vegascene(specFilePath, outFilePath);
        return EXIT_SUCCESS;
    }
}
