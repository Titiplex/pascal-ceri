//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <iostream>
#include <getopt.h>
#include <string>
#undef BEGIN
#undef END
#include <parser.h>

#include <filesystem>
#include <fstream>

#include "FlexLexer.h"
#include "tokeniser.h"
#include "utils.h"

void Program()
{
    // Header for gcc assembler / linker
    std::cout << "\t\t\t# This code was produced by the CERI Compiler" << std::endl;
    std::cout << "\t.data" << std::endl;
    std::cout << "\t.align 8" << std::endl;

    std::cout << "FormatStringInt:\t.string \"%llu\\n\"" << std::endl; // le display
    std::cout << "FormatStringDouble:\t.string \"%f\\n\"" << std::endl; // le display
    std::cout << "FormatStringChar:\t.string \"%c\\n\"" << std::endl; // le display
    std::cout << "FormatStringString:\t.string \"%s\\n\"" << std::endl;
    std::cout << "TrueString:\t.string \"TRUE\\n\"" << std::endl;
    std::cout << "FalseString:\t.string \"FALSE\\n\"" << std::endl;
    next();
    if (getCurrent() == KEYWORD && getCurrentKeyword() == VAR)
        VarDeclarationPart();
    if (getCurrent() == KEYWORD && (getCurrentKeyword() == FUNCTION || getCurrentKeyword() == PROCEDURE))
        FunctionDeclarationPart();
    StatementPart();

    // Trailer for the gcc assembler / linker
    std::cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top" << std::endl;
    std::cout << "\txor     %eax, %eax\t\t# 0 output code" << std::endl;
    std::cout << "\tret\t\t\t# Return from main function" << std::endl;
    if (getCurrent() != FEOF)
    {
        std::cerr << "Caractères en trop à la fin du programme : [" << getCurrent() << "]";
        Error("."); // unexpected characters at the end of the program
    }
}

static void help(const char* invoke)
{
    std::cout << "Usage:" << std::endl
        << "\t" << invoke << " [options] [fichiers...]" << std::endl << std::endl
        << "Options:" << std::endl
        << "\t-h, --help          Affiche cette aide" << std::endl
        << "\t-o, --output <f>    Ecrit la sortie assembleur dans <f>" << std::endl;
}

static std::string getExtension(const std::string& filename)
{
    const unsigned long long pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    return filename.substr(pos + 1);
}

extern FlexLexer* lexer;
extern "C" {
    extern FILE* yyin; // input flex pour la redirection de l'input
}

int main(const int argc, char** argv)
{
    if (argc == 1)
    {
        std::cerr << "Erreur : absence de fichier à compiler, utilisez '--help'";
        return EXIT_FAILURE;
    }

    std::string outputPath;

    // options de compilation
    // https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
    static option long_opts[] = {
        { "help",    no_argument,       nullptr, 'h' },
        { "output",  optional_argument, nullptr, 'o' },
        { nullptr,   0,                 nullptr,  0  }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "ho:", long_opts, nullptr))!=-1)
    {
        switch (opt)
        {
            case 'o':
                outputPath = optarg;
                break;
            case 'h':
            case '?': // unknown option
            default:
                help(argv[0]);
        }
    }

    if (argc - optind > 1 && !outputPath.empty()) {
        std::cerr << "Erreur : option -o incompatible avec plusieurs fichiers." << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = optind; i < argc; ++i)
    {
        std::ifstream f(argv[i], std::ios::in);
        if (!f)
        {
            std::cerr << "Erreur : impossible d'ouvrir le fichier '" << argv[i] << "'" << std::endl;
            perror(argv[i]);
            return EXIT_FAILURE;
        }
        std::string ext = getExtension(argv[i]);
        if (ext != "pas" && ext != "p")
        {
            std::cerr << "/!\\ Attention : le fichier '" << argv[i] << "' n'est peut-etre pas un fichier de langage Pascal" << std::endl;
        }
        if (outputPath.empty())
        {
            std::string filename = argv[i];
            outputPath = filename.erase(filename.find(ext), ext.size()).append("s");
        }
        std::ofstream output(outputPath, std::ios::out | std::ios::trunc);
        if (!output) {
            std::cerr << "Erreur : impossible d'ouvrir " << outputPath << std::endl;
            return EXIT_FAILURE;
        }

        lexer->switch_streams(&f, nullptr);
        std::string content = captureOutputOf([] {Program();});
        output << "# " << argv[i] << std::endl << content << std::endl;
        if (!std::filesystem::is_empty(outputPath))
        {
            std::cerr << "Erreur : impossible d'ecrire dans " << outputPath << std::endl;
            return EXIT_FAILURE;
        }
        f.close();
        output.close();
        // std::cout << content;
    }

    return EXIT_SUCCESS;
}
