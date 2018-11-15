# neon-diff [![TravisCI build](https://travis-ci.org/maxrd2/neon-diff.svg?branch=master)](https://travis-ci.org/maxrd2/neon-diff)

`neon-diff` is an application written in C++ that colorizes and highlights unified diffs similar to colordiff, but using **smart highlighting of changed words/characters**. Its goal is to make you spot changes in big chunks of code/text much faster and to make diffs more readable.

- Colorizes changed lines and headers (similar to git)
- Detects changed parts of lines and highlights them 
- Output remains valid unified diff (it can be used with git's interactive.diffFilter)

Screenshot of vanilla `git diff` compared to `git` and `neon-diff`
![screenshot-git-vs-neon](https://user-images.githubusercontent.com/1187381/48526699-e7fb4300-e888-11e8-8d46-706f5084f5f7.png)

## Build/Install

You will require git, cmake and gcc (or clang) compiler.
```shell
git clone https://github.com/maxrd2/neon-diff.git && cd neon-diff
mkdir build && cd build
cmake ..
make
sudo make install
```

## Usage

#### git

```shell
git diff | neon-diff | less --tabs=4 -RFX
```

#### diff

```shell
diff --unified originalFile.txt changedFile.txt | neon-diff | less --tabs=4 -RFX
```

#### configuring as default in git

Configure git to use `neon-diff` for all diff output:
```shell
git config --global core.pager "neon-diff | less --tabs=4 -RFX"
```

Configure git to use `neon-diff` for interactive add (git add -pi):
```shell
git config --global interactive.diffFilter "neon-diff"
```

## Contributing

Pull requests and patches are welcome. Please follow the [coding style](README.CodingStyle.md).

We are also looking for any feedback or ideas on how to make neon-diff even better.

## License

`neon-diff` is released under [GNU General Public License v3.0](LICENSE)
