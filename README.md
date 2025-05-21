# OpenAnimeCLI

OpenAnimeCLI, OpenAnime icin bir komut satırı istemcisidir. Bu istemci, OpenAnime API'sini kullanarak Turkce anime izlemenizi saglar.

OpenAnimeCLI'in OpenAnime Labs. ile hicbir baglantisi yoktur. Bu proje tamamiyla topluluk destekli bir projedir.

## NOT

Bu proje hala gelistirilme asamasindadir!!!!!!!!!!!!!!1

## Kurulum

```bash
git clone https://github.com/XielQs/OpenAnimeCLI
cd OpenAnimeCLI
mkdir build
cd build
cmake ..
cmake --build . -- -j$(nproc)
```

Calistirilabilir dosya `build/` dizininde bulunmaktadir.

## Kullanım

```bash
> ./openanime-cli --help
Kullanim: openanime-cli [secenekler]
Secenekler:
  --help       Bu yardimi goster
  --version    Versiyon bilgisini goster
  --fzf        FZF ile secim yap
```

## Lisans

Bu proje MIT lisansı altında lisanslanmıştır. Lisans metni için [LICENSE](LICENSE) dosyasına bakabilirsiniz.
