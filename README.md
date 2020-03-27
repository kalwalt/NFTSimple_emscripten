# NFTSimple_emscripten


## Introduction


The Basic NFT Simple **artoolkit5** example ported to js with **emscripten**.
The project is a WIP.
The initial idea was to simply port the nftSImple.c example to js with [emscripten](https://emscripten.org).
For different reason i found that this simplicity is not possible (sigh!),
i am trying to develop a mixed solution with javascript 'injected' with EM_ASM and compiled JS by emscripten... maybe with --pre-js also.
You can follow the development in the `dev` branch, see this [PR](https://github.com/kalwalt/NFTSimple_emscripten/pull/1)
My goal is to test the threaded version for the NFT marker. The idea started because i'm working on the [jsartoolkit5 NFT branch](https://github.com/kalwalt/jsartoolkit5/pull/1)

## Code infos

I use emmscripten 1.38.31 and artoolkit5.
OS: Ubuntu 16.04.2