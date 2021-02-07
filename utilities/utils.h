
namespace fs = boost::filesystem;



std::vector<fs::path> get_all(fs::path const & root, std::string const & ext)
{
    std::vector<fs::path> paths;

    if (fs::exists(root) && fs::is_directory(root))
    {
        for (auto const & entry : fs::recursive_directory_iterator(root))
        {
            if (fs::is_regular_file(entry) && entry.path().extension() == ext){
                paths.emplace_back(entry.path().filename());
	    }
        }
    }
    return paths;
}

