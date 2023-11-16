# vga

> 开源VGA模块，支持的输出分辨率有*800x600*, _640x480_, _480x272_, _320x240_，默认支持*640x480*;
> 支持Core选择分辨率、自测试模式输出彩条画面、配置SDRAM读取地址

## 使用本仓库

1. 测试vga_top功能: `make`
   > PS: 更改Makefile里的module，可以单独对模块进行测试
2. 查看波形: `make wave`
3. 在SDL里展示彩条画面: `make sdl`
   > PS: 需要保证按照了SDL2

## 各版本设计文档

1. [🌟🌟🌟🌟vga-v5(最终版)](docs/vga-v5.md)
2. [vga-v4](docs/vga-v4.md)
3. [vga-v3](docs/vga-v3.md)
4. [vga-v2](docs/vga-v2.md)
5. [vga-v1](docs/vga-v1.md)

## 代码仓库

[代码地址](./src/rtl/)
