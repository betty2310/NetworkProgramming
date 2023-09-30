## Network Programming `IT4062`


### Using docker 🐳

The environment required by this project is run on Ubuntu:22.04, so if you
don't like Ubuntu like me (I use Arch, btw). Docker is best for now.

```shell
docker build -t c-dev-env .
docker run -it -v .:/network_programming c-dev-env
```

### Debug 🐛
Support debug with `gdb` in container. I use [Visual Studio Code](https://code.visualstudio.com/) to debug, so you can see
`w_/.vscode/launch.json` for more detail.

Or just press <kbd>F5</kbd> in VSCode to start debug.