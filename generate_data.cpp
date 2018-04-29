#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <random>

using std::shuffle;
using std::vector;
using std::cout;
using std::endl;

struct {

    std::string out_file_name;
    size_t num = 10000;

} pr_opts;


void parse_args(int argc, char* argv[])
{
    int opt = 0;

    const char* opt_string = "o:n:";

    while ( (opt = getopt(argc, argv, opt_string))  != -1 )
    {
        switch (opt)
        {
            case 'o':
                pr_opts.out_file_name = optarg;
                break;

            case 'n':
                pr_opts.num = static_cast<size_t>(strtoll(optarg, nullptr, 10));
                break;

            default:
                break;
        }
    }
}

void generate_keys(std::vector<size_t>& keys)
{
    std::random_device rd;
    std::mt19937 g{rd()};
    for (size_t i = 0; i < keys.size(); i++)
    {
        keys[i] = i;
    }
    shuffle(keys.begin(), keys.end(), g);
}

void write_data(std::ofstream& out, std::vector<size_t>& keys)
{
    std::hash<std::string> hasher;
    cout << "Generating data..." << endl;
    for (size_t& key : keys)
    {
        out << "%%%" << std::dec << key << ";" << std::hex << hasher(std::to_string(key)) << "$$$" << endl;
    }
    cout << "done." << endl;
}



int main(int argc, char* argv[])
{
    parse_args(argc, argv);

    try
    {
        vector<size_t> keys{};
        keys.resize(pr_opts.num);
        generate_keys(keys);

        std::ofstream out{pr_opts.out_file_name};
        write_data(out, keys);
    }
    catch (std::exception& ex)
    {
        cout << ex.what() << endl;
    }

    return 0;
}