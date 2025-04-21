<p align="center">
  <img src="https://avatars.githubusercontent.com/u/138057124?s=200&v=4" width="150" />
</p>
<h1 align="center">Byte Pair Encoding</h1>

<p align="center">
  
</p>

<h4 align="center">
  <a href="https://github.com/WillKirkmanM/music/releases">Releases</a>
</h4>

<p align="center">The Large Language Model Tokenizer Algorithm</p>


This project provides a command-line tool implemented in C++ for training a Byte Pair Encoding (BPE) model on a text corpus and encoding text using the learned model.

## How BPE Works

Byte Pair Encoding is a data compression technique that is commonly used in Natural Language Processing (NLP) for tokenisation. It helps manage large vocabularies and handle unknown words.

Here's a simplified overview of the BPE **training** process:

1.  **Initialisation**:
    *   Start with a vocabulary consisting of all individual characters (or bytes) present in the training corpus.
    *   Represent the corpus as a sequence of these initial character/byte tokens.

2.  **Iteration**:
    *   Count the frequency of all adjacent pairs of tokens in the current sequence.
    *   Identify the most frequent pair (e.g., 't' followed by 'h').
    *   **Merge** this most frequent pair into a single new token (e.g., 'th').
    *   Add this new token to the vocabulary.
    *   Replace all occurrences of the original pair in the sequence with the new merged token.

3.  **Repeat**:
    *   Repeat the iteration step (counting, finding the most frequent pair, merging) for a predetermined number of merges or until the desired vocabulary size is reached.

The result of training is:
*   A **vocabulary** containing the initial characters/bytes and the new merged tokens.
*   An ordered list of **merge rules** indicating which pairs were merged to create which new tokens.

**Encoding** new text involves:
1.  Splitting the text into its initial character/byte sequence.
2.  Applying the learned merge rules *in the same order they were learned during training* to the sequence until no more merges can be applied.
3.  The final sequence of tokens (original characters/bytes and merged tokens) is the BPE-encoded representation.

## Building the Tool

This project uses CMake to generate build files for various build systems like Make and Ninja.

**Prerequisites:**
*   A C++17 compliant compiler (like g++, Clang, or MSVC)
*   CMake (version 3.10 or higher)
*   A build tool (like `make` or `ninja`)

**Steps:**

1.  **Clone/Download:** Get the project files.
2.  **Create Build Directory:**
    ```bash
    cd byte-pair-encoding
    mkdir build
    cd build
    ```
3.  **Configure with CMake:**
    *   **For Makefiles (Default on many Linux/macOS systems):**
        ```bash
        cmake ..
        ```
    *   **For Ninja (Often faster):**
        ```bash
        cmake -G Ninja ..
        ```
    *   **For Visual Studio (Windows):** Open the folder in Visual Studio, or use CMake GUI, or run from a Developer Command Prompt:
        ```cmd
        # Example for VS 2019
        cmake -G "Visual Studio 16 2019" -A x64 ..
        ```
4.  **Build:**
    *   **If using Make:**
        ```bash
        make
        ```
    *   **If using Ninja:**
        ```bash
        ninja
        ```
    *   **If using Visual Studio:** Build the solution (`bpe_tool.sln`) within the IDE or use MSBuild:
        ```cmd
        msbuild bpe_tool.sln /property:Configuration=Release
        ```

The executable `bpe_tool` (or `bpe_tool.exe` on Windows) will be created in the `build` directory.

## Running the Tool

Place your input text file (e.g., `shakespeare.txt`) in the main project directory. Run the tool from the `build` directory or copy the executable elsewhere.

**1. Train a BPE Model:**

```bash
# Usage: ./bpe_tool train <input_file> <vocab_size> <output_merges_file>
./bpe_tool train ../shakespeare.txt 1000 ../shakespeare.merges
```
*   `<input_file>`: Path to the training text (e.g., `../shakespeare.txt`).
*   `<vocab_size>`: The target total vocabulary size (initial 256 bytes + number of merges). Must be > 256. Example: `1000`.
*   `<output_merges_file>`: Path where the learned merge rules will be saved (e.g., `../shakespeare.merges`).

**2. Encode Text using a Trained Model:**

```bash
# Usage: ./bpe_tool encode <input_file> <merges_file>
./bpe_tool encode ../my_text_to_encode.txt ../shakespeare.merges
```
*   `<input_file>`: Path to the text you want to encode (e.g., `../my_text_to_encode.txt`). Create this file with some sample text.
*   `<merges_file>`: Path to the merge rules file created during training (e.g., `../shakespeare.merges`).

The tool will print the resulting sequence of token IDs to the console.
