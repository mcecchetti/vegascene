
#include "vegascene.h"

#include <iostream>

#include <vector>

#include <QGuiApplication>


void printUsage()
{
    std::cout << "Usage:\n"
              << "  vegascene vega_json_file [output_json_file]\n"
              << "  If output_json_file is not provided, writes to stdout.\n\n"
              << "To load data, you may need to set a base directory:\n"
              << "  For web retrieval, use `-b http://host/data/`. \n"
              << "  For files, use `-b file:///dir/data/` (absolute) or `-b data/` (relative).\n";
}

int main(int argc, char *argv[])
{
    // for font metrics support
    QGuiApplication app(argc, argv);

    String optionBase("-b");
    String baseURL("");

    std::vector<String> argList;

    for (int i = 1; i < argc; ++i)
    {
        String arg( argv[i] );
        if( arg == optionBase )
        {
            ++i;
            if (i < argc)
            {
                baseURL = String(argv[i]);
            }
            else
            {
                printUsage();
                return EXIT_FAILURE;
            }
        }
        else
        {
            argList.push_back(arg);
        }
    }

    if (argList.size() < 1 || argList.size() > 2)
    {
        printUsage();
        return EXIT_FAILURE;
    }
    else if (argList.size() == 1)
    {
        argList.push_back(String(""));
    }

    return vegascene(argList[0], argList[1], baseURL);
}
