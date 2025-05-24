# OpenAnimeCLI

OpenAnimeCLI, OpenAnime API'sini kullanarak istediginiz animeyi Turkce bir sekilde izlemenizi saglayan bir komut satiri aracidir.

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

## Kullanim

```bash
> ./openanime-cli --help
Kullanim: openanime-cli [anime_adi] [secenekler]
Secenekler:
  --help       Bu yardimi goster
  --version    Versiyon bilgisini goster
  --fzf        FZF ile secim yap
```

## Lisans

Bu proje MIT lisansi ile lisanslanmistir. Lisans bilgileri icin `LICENSE` dosyasina bakabilirsiniz.
