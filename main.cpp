#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>
#include <filesystem>

using MergeRule = std::pair<std::pair<int, int>, int>;

std::map<std::pair<int, int>, int> get_stats(const std::vector<int>& ids) {
    std::map<std::pair<int, int>, int> counts;
    if (ids.size() < 2) {
        return counts;
    }
    for (size_t i = 0; i + 1 < ids.size(); ++i) {
        counts[{ids[i], ids[i + 1]}]++;
    }
    return counts;
}

std::vector<int> merge(const std::vector<int>& ids, const std::pair<int, int>& pair_to_merge, int new_id) {
    std::vector<int> new_ids;
    new_ids.reserve(ids.size());
    size_t i = 0;
    while (i < ids.size()) {
        if (i + 1 < ids.size() && ids[i] == pair_to_merge.first && ids[i + 1] == pair_to_merge.second) {
            new_ids.push_back(new_id);
            i += 2;
        } else {
            new_ids.push_back(ids[i]);
            i += 1;
        }
    }
    return new_ids;
}

std::vector<MergeRule> train_bpe(const std::string& text, int vocab_size) {
    std::cout << "Starting BPE training..." << std::endl;
    std::vector<MergeRule> merges;
    if (text.empty()) {
        std::cerr << "Warning: Input text for training is empty." << std::endl;
        return merges;
    }

    std::vector<int> ids;
    ids.reserve(text.length());
    for (unsigned char c : text) {
        ids.push_back(static_cast<int>(c));
    }

    int num_merges = vocab_size - 256;
    if (num_merges <= 0) {
         std::cerr << "Warning: vocab_size (" << vocab_size << ") must be greater than 256. No merges will be performed." << std::endl;
         return merges;
    }
    merges.reserve(num_merges);

    int next_id = 256;

    for (int i = 0; i < num_merges; ++i) {
        std::map<std::pair<int, int>, int> stats = get_stats(ids);

        if (stats.empty()) {
            std::cout << "No more pairs to merge. Stopping training early." << std::endl;
            break;
        }

        auto max_pair_it = std::max_element(stats.begin(), stats.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        std::pair<int, int> pair_to_merge = max_pair_it->first;

        ids = merge(ids, pair_to_merge, next_id);

        merges.push_back({pair_to_merge, next_id});

        std::cout << "Merge " << (i + 1) << "/" << num_merges
                  << ": (" << pair_to_merge.first << ", " << pair_to_merge.second << ") -> "
                  << next_id << " (Frequency: " << max_pair_it->second << ")" << std::endl;

        next_id++;
    }

    std::cout << "BPE training finished. Learned " << merges.size() << " merges." << std::endl;
    return merges;
}

bool save_merges(const std::string& filename, const std::vector<MergeRule>& merges) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Cannot open file to save merges: " << filename << std::endl;
        return false;
    }
    for (const auto& merge_rule : merges) {
        outfile << merge_rule.first.first << " " << merge_rule.first.second << " " << merge_rule.second << "\n";
    }
    outfile.close();
    std::cout << "Merges saved to " << filename << std::endl;
    return true;
}

std::vector<MergeRule> load_merges(const std::string& filename) {
    std::vector<MergeRule> merges;
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open file to load merges: " << filename << std::endl;
        return merges;
    }
    int id1, id2, new_id;
    std::string line;
    while (std::getline(infile, line)) {
         std::stringstream ss(line);
         if (ss >> id1 >> id2 >> new_id) {
             merges.push_back({{id1, id2}, new_id});
         } else {
              std::cerr << "Warning: Skipping invalid line in merges file: " << line << std::endl;
         }
    }
    infile.close();
    std::cout << "Merges loaded from " << filename << std::endl;
    return merges;
}

std::vector<int> encode(const std::string& text, const std::vector<MergeRule>& merges) {
    std::cout << "Starting encoding..." << std::endl;
    if (text.empty()) {
         std::cerr << "Warning: Input text for encoding is empty." << std::endl;
         return {};
    }

    std::vector<int> ids;
     ids.reserve(text.length());
    for (unsigned char c : text) {
        ids.push_back(static_cast<int>(c));
    }

    for (const auto& rule : merges) {
        const std::pair<int, int>& pair_to_merge = rule.first;
        int new_id = rule.second;
        std::vector<int> next_ids;
        next_ids.reserve(ids.size());
        size_t i = 0;
        while (i < ids.size()) {
            if (i + 1 < ids.size() && ids[i] == pair_to_merge.first && ids[i + 1] == pair_to_merge.second) {
                next_ids.push_back(new_id);
                i += 2;
            } else {
                next_ids.push_back(ids[i]);
                i += 1;
            }
        }
        ids = std::move(next_ids);
    }

    std::cout << "Encoding finished." << std::endl;
    return ids;
}

std::string read_file(const std::string& filename) {
    std::ifstream infile(filename, std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        exit(1);
    }

    std::ostringstream buffer;
    buffer << infile.rdbuf();
    infile.close();
    return buffer.str();
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n";
        std::cerr << "  " << argv[0] << " train [<input_file>] <vocab_size> <output_merges_file>\n";
        std::cerr << "  " << argv[0] << " encode [<input_file>] <merges_file>\n";
        std::cerr << "Default <input_file> is 'shakespeare.txt' if omitted.\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string default_input_filename = "shakespeare.txt";
    std::string input_filename;

    if (mode == "train") {
        int vocab_size = 0;
        std::string output_merges_filename;

        if (argc == 5) {
            input_filename = argv[2];
            try {
                vocab_size = std::stoi(argv[3]);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid vocab_size '" << argv[3] << "'. Must be an integer." << std::endl;
                return 1;
            }
            output_merges_filename = argv[4];
        } else if (argc == 4) {
            input_filename = default_input_filename;
            try {
                vocab_size = std::stoi(argv[2]);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid vocab_size '" << argv[2] << "'. Must be an integer." << std::endl;
                return 1;
            }
            output_merges_filename = argv[3];
            std::cout << "Input file not specified, defaulting to '" << default_input_filename << "'" << std::endl;
        } else {
            std::cerr << "Usage: " << argv[0] << " train [<input_file>] <vocab_size> <output_merges_file>\n";
            std::cerr << "Default <input_file> is 'shakespeare.txt' if omitted.\n";
            return 1;
        }

        if (!std::filesystem::exists(input_filename)) {
            std::cerr << "Error: Input file not found: " << input_filename << std::endl;
            return 1;
        }

        std::string text = read_file(input_filename);
        std::vector<MergeRule> merges = train_bpe(text, vocab_size);
        if (!save_merges(output_merges_filename, merges)) {
            return 1;
        }
        std::cout << "Training complete. Merges saved to " << output_merges_filename << std::endl;

    } else if (mode == "encode") {
        std::string merges_filename;

        if (argc == 4) {
            input_filename = argv[2];
            merges_filename = argv[3];
        } else if (argc == 3) {
            input_filename = default_input_filename;
            merges_filename = argv[2];
            std::cout << "Input file not specified, defaulting to '" << default_input_filename << "'" << std::endl;
        } else {
            std::cerr << "Usage: " << argv[0] << " encode [<input_file>] <merges_file>\n";
            std::cerr << "Default <input_file> is 'shakespeare.txt' if omitted.\n";
            return 1;
        }

        if (!std::filesystem::exists(input_filename)) {
            std::cerr << "Error: Input file not found: " << input_filename << std::endl;
            return 1;
        }

        std::vector<MergeRule> merges = load_merges(merges_filename);
        if (merges.empty()) {
            if (std::filesystem::exists(merges_filename)) {
                std::cerr << "Warning: Loaded merges file is empty or invalid: " << merges_filename << std::endl;
            } else {
                std::cerr << "Error: Cannot open file to load merges: " << merges_filename << std::endl;
                return 1;
            }
        }

        std::string text = read_file(input_filename);
        std::vector<int> encoded_ids = encode(text, merges);

        std::cout << "\nEncoded Token IDs:" << std::endl;
        for (size_t i = 0; i < encoded_ids.size(); ++i) {
            std::cout << encoded_ids[i] << (i == encoded_ids.size() - 1 ? "" : " ");
        }
        std::cout << std::endl;

    } else {
        std::cerr << "Error: Unknown mode '" << mode << "'. Use 'train' or 'encode'." << std::endl;
        std::cerr << "Usage:\n";
        std::cerr << "  " << argv[0] << " train [<input_file>] <vocab_size> <output_merges_file>\n";
        std::cerr << "  " << argv[0] << " encode [<input_file>] <merges_file>\n";
        std::cerr << "Default <input_file> is 'shakespeare.txt' if omitted.\n";
        return 1;
    }

    return 0;
}