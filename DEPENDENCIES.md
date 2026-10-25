# Dependencies

| Component | Version / revision | Source | License |
| --- | --- | --- | --- |
| LVGL | v9.2.2 / 7f07a129e8d77f4984fff8e623fd5be18ff42e74 | https://github.com/lvgl/lvgl | MIT |
| SDL2 | Distribution-provided 2.x; tested runtime 2.30.0 | https://github.com/libsdl-org/SDL | zlib |
| CMake | >=3.20 | https://cmake.org/ | BSD-3-Clause |
| C/C++ compiler and Make or Ninja | Distribution-provided | Distribution packages | Respective upstream licenses |

LVGL download SHA-256:

`8a5e30223fd5b08cdeccb060f45b37a29868d6b1fdfc5bdbdf0dd89aaefb5a0e`

Archive: https://codeload.github.com/lvgl/lvgl/tar.gz/7f07a129e8d77f4984fff8e623fd5be18ff42e74

Sources are fetched into the build directory; third-party source trees and binaries
are not included in the project ZIP. The upstream license texts are retained in
`third_party/LVGL-LICENSE.txt` and `third_party/SDL2-LICENSE.txt`.
