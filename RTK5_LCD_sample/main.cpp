//=====================================================================//
/*! @file
    @brief  RX65N ＬＣＤ描画実験用コード
    @author 平松邦仁 (hira@rvf-rc45.net)
	@copyright	Copyright (C) 2018 Kunihito Hiramatsu @n
				Released under the MIT license @n
				https://github.com/hirakuni45/RX/blob/master/LICENSE
*/
//=====================================================================//
// 漢字フォントをＳＤカードから読んでキャッシュする場合
#define CASH_KFONT

#include "common/renesas.hpp"
#include "common/fixed_fifo.hpp"
#include "common/sci_io.hpp"
#include "common/format.hpp"
#include "common/command.hpp"
#include "common/shell.hpp"
#include "common/spi_io2.hpp"
#include "common/qspi_io.hpp"
#include "graphics/font8x16.hpp"
#include "graphics/font.hpp"
#include "graphics/graphics.hpp"

// #include "graphics/jpeg_in.hpp"
#include "graphics/bmp_in.hpp"
#include "graphics/filer.hpp"
#include "graphics/dialog.hpp"
#include "chip/FT5206.hpp"

#include "graphics/picojpeg_in.hpp"
#include "graphics/img_in.hpp"
#include "graphics/scaling.hpp"

// #define SOFT_I2C
#ifdef SOFT_I2C
#include "common/si2c_io.hpp"
#else
#include "common/sci_i2c_io.hpp"
#endif

// SDHI インターフェースを使う場合
// #define SDHI_IF

// DRW2D エンジンを使う場合
#define USE_DRW2D


#include "usb/usb_io.hpp"
#include "usb/usb_hmsc.hpp"

#include "graphics/tgl.hpp"

namespace {

	typedef device::PORT<device::PORT7, device::bitpos::B0> LED;
	typedef device::PORT<device::PORT0, device::bitpos::B5> SW2;

	typedef device::system_io<12000000> SYSTEM_IO;

	typedef utils::fixed_fifo<char,  512> RECV_BUFF;
	typedef utils::fixed_fifo<char, 1024> SEND_BUFF;
	typedef device::sci_io<device::SCI9, RECV_BUFF, SEND_BUFF> SCI;
	SCI			sci_;

	// ＳＤカード電源制御は使わない場合、「device::NULL_PORT」を指定する。
//	typedef device::PORT<device::PORT6, device::bitpos::B4> SDC_POWER;
	typedef device::NULL_PORT SDC_POWER;

#ifdef SDHI_IF
	typedef fatfs::sdhi_io<device::SDHI, SDC_POWER, device::port_map::option::THIRD> SDHI;
	SDHI		sdh_;
#else
	// Soft SDC 用　SPI 定義（SPI）
	typedef device::PORT<device::PORT2, device::bitpos::B2> MISO;  // DAT0
	typedef device::PORT<device::PORT2, device::bitpos::B0> MOSI;  // CMD
	typedef device::PORT<device::PORT2, device::bitpos::B1> SPCK;  // CLK

	typedef device::spi_io2<MISO, MOSI, SPCK> SPI;  ///< Soft SPI 定義

	SPI			spi_;

	typedef device::PORT<device::PORT1, device::bitpos::B7> SDC_SELECT;  // DAT3 カード選択信号
	typedef device::PORT<device::PORT2, device::bitpos::B5> SDC_DETECT;  // CD   カード検出

	typedef fatfs::mmc_io<SPI, SDC_SELECT, SDC_POWER, SDC_DETECT> MMC;   // ハードウェアー定義

	MMC			sdh_(spi_, 35000000);
#endif

	typedef device::PORT<device::PORT6, device::bitpos::B3> LCD_DISP;
	typedef device::PORT<device::PORT6, device::bitpos::B6> LCD_LIGHT;
	static const int16_t LCD_X = 480;
	static const int16_t LCD_Y = 272;
	static void* LCD_ORG = reinterpret_cast<void*>(0x00000100);
	static const auto PXT = graphics::pixel::TYPE::RGB565;
	typedef device::glcdc_io<device::GLCDC, LCD_X, LCD_Y, PXT> GLCDC_IO;
	GLCDC_IO	glcdc_io_(nullptr, LCD_ORG);

	typedef graphics::font8x16 AFONT;
	AFONT		afont_;
	typedef graphics::kfont<16, 16, 64> KFONT;
	KFONT		kfont_;
	typedef graphics::font<AFONT, KFONT> FONT;
	FONT		font_(afont_, kfont_);

	typedef graphics::def_color DEF_COLOR;

#ifdef USE_DRW2D
	typedef device::drw2d_mgr<GLCDC_IO, FONT> RENDER;
#else
	typedef graphics::render<GLCDC_IO, FONT> RENDER;
#endif
	RENDER		render_(glcdc_io_, font_);

//	typedef graphics::filer<SDC, RENDER> FILER;
//	FILER		filer_(render_);

	// FT5206, SCI6 簡易 I2C 定義
	typedef device::PORT<device::PORT0, device::bitpos::B7> FT5206_RESET;
#ifdef SOFT_I2C
	typedef device::PORT<device::PORT0, device::bitpos::B0> FT5206_SDA;
	typedef device::PORT<device::PORT0, device::bitpos::B1> FT5206_SCL;
	typedef device::si2c_io<FT5206_SDA, FT5206_SCL> FT5206_I2C;
#else
	typedef utils::fixed_fifo<uint8_t, 64> RECV6_BUFF;
	typedef utils::fixed_fifo<uint8_t, 64> SEND6_BUFF;
	typedef device::sci_i2c_io<device::SCI6, RECV6_BUFF, SEND6_BUFF,
			device::port_map::option::FIRST_I2C> FT5206_I2C;
#endif
	FT5206_I2C	ft5206_i2c_;
	typedef chip::FT5206<FT5206_I2C> FT5206;
	FT5206		ft5206_(ft5206_i2c_);

	typedef gui::dialog<RENDER, FT5206> DIALOG;
	DIALOG		dialog_(render_, ft5206_);

#if 0
	typedef img::scaling<RENDER> PLOT;
	PLOT		plot_(render_);
	typedef img::img_in<PLOT> IMG_IN;
	IMG_IN		imgs_(plot_);
#endif

	// QSPI B グループ
	typedef device::qspi_io<device::QSPI, device::port_map::option::SECOND> QSPI;
	QSPI		qspi_;

	typedef utils::command<256> CMD;
	CMD			cmd_;
	typedef utils::shell<CMD> SHELL;
	SHELL		shell_(cmd_);


	void command_()
	{
		if(!cmd_.service()) {
			return;
		}
		if(shell_.analize()) {
			return;
		}

		auto cmdn = cmd_.get_words();
		if(cmd_.cmp_word(0, "image")) { // image load, draw
			if(cmdn >= 2) {
				char tmp[128];
				cmd_.get_word(1, tmp, sizeof(tmp));
///				imgs_.load(tmp);
			}
		} else if(cmd_.cmp_word(0, "help")) {
			shell_.help();
			utils::format("    image [filename]\n");
		} else {
			utils::format("Command error: '%s'\n") % cmd_.get_command();
		}
	}
}

extern "C" {

	void sci_putch(char ch)
	{
		sci_.putch(ch);
	}


	void sci_puts(const char* str)
	{
		sci_.puts(str);
	}


	char sci_getch(void)
	{
		return sci_.getch();
	}


	uint16_t sci_length()
	{
		return sci_.recv_length();
	}


	DSTATUS disk_initialize(BYTE drv) {
		return sdh_.disk_initialize(drv);
	}


	DSTATUS disk_status(BYTE drv) {
		return sdh_.disk_status(drv);
	}


	DRESULT disk_read(BYTE drv, BYTE* buff, DWORD sector, UINT count) {
		return sdh_.disk_read(drv, buff, sector, count);
	}


	DRESULT disk_write(BYTE drv, const BYTE* buff, DWORD sector, UINT count) {
		return sdh_.disk_write(drv, buff, sector, count);
	}


	DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void* buff) {
		return sdh_.disk_ioctl(drv, ctrl, buff);
	}


	DWORD get_fattime(void) {
		time_t t = 0;
///		rtc_.get_time(t);
		return utils::str::get_fattime(t);
	}


#if 0
	void gr_plot(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b)
	{
		uint16_t c =   (static_cast<uint16_t>(r & 0xf8) << 8)
              | (static_cast<uint16_t>(g & 0xfc) << 3)
              | (static_cast<uint16_t>(b & 0xf8) >> 3);
		render_.plot(x, y, c);
	}
#endif
}

int main(int argc, char** argv);

int main(int argc, char** argv)
{
	SYSTEM_IO::setup_system_clock();

	{  // SCI 設定
		static const uint8_t sci_level = 2;
		sci_.start(115200, sci_level);
	}

	{  // SD カード・クラスの初期化
		sdh_.start();
	}

	utils::format("\rRTK5RX65N Start for LCD sample\n");

	cmd_.set_prompt("# ");

	{  // QSPI の初期化（Flash Memory Read/Write Interface)
		if(!qspi_.start(1000000, QSPI::PHASE::TYPE1, QSPI::DLEN::W8)) {
			utils::format("QSPI not start.\n");
		}
	}

	{  // GLCDC の初期化
		LCD_DISP::DIR  = 1;
		LCD_LIGHT::DIR = 1;
		LCD_DISP::P  = 0;  // DISP Disable
		LCD_LIGHT::P = 0;  // BackLight Disable (No PWM)
		if(glcdc_io_.start()) {
			utils::format("Start GLCDC\n");
			LCD_DISP::P  = 1;  // DISP Enable
			LCD_LIGHT::P = 1;  // BackLight Enable (No PWM)
			if(!glcdc_io_.control(GLCDC_IO::CONTROL_CMD::START_DISPLAY)) {
				utils::format("GLCDC ctrl fail...\n");
			}
		} else {
			utils::format("GLCDC Fail\n");
		}
	}

	{  // DRW2D 初期化
//		render_.list_info();
		if(render_.start()) {
			utils:: format("Start DRW2D\n");
		} else {
			utils:: format("DRW2D Fail\n");
		}
	}

	{  // FT5206 touch screen controller
		FT5206::reset<FT5206_RESET>();
		uint8_t intr_lvl = 1;
		if(!ft5206_i2c_.start(FT5206_I2C::SPEED::STANDARD, intr_lvl)) {
			utils::format("FT5206 I2C Start Fail...\n");
		}
		if(!ft5206_.start()) {
			utils::format("FT5206 Start Fail...\n");
		}
	}

	LED::DIR = 1;
	SW2::DIR = 0;

	uint8_t task = 100;
	FT5206::xy	pos;

	bool sw2 = SW2::P();
	uint8_t n = 0;

	uint16_t rad = 10;
	uint16_t render_task = 0;
	while(1) {
		render_.sync_frame();
		ft5206_.update();
		sdh_.service();

#if 0
		{
			uint8_t ctrl = 0;

			auto tnum = ft5206_.get_touch_num();
			const auto& xy = ft5206_.get_touch_pos(0);
			filer_.set_touch(tnum, xy.x, xy.y); 
			char path[256];
			if(filer_.update(ctrl, path, sizeof(path))) {

			}
		}
#endif

#if 0
		if(task > 0) {
			--task;
			if(task == 0) {
				img::jpeg_in jpeg;
				utils::file_io fin;
				if(fin.open("aaaaa.jpg", "rb")) {
					if(!jpeg.load(fin)) {
						utils::format("JPEG load fail...\n");
					}
					fin.close();
				}
				char tmp[32];
				for(int i = 0; i < 26; ++i) tmp[i] = 'A' + i;
				tmp[26] = 0;
				render_.draw_text(0, 0, tmp);
				for(int i = 0; i < 26; ++i) tmp[i] = 'a' + i;
				tmp[26] = 0;
				render_.draw_text(0, 16, tmp);
				render_.draw_text(0, 32, "金の貸し借りをしてはならない。\n金を貸せば金も友も失う。\n金を借りれば倹約が馬鹿らしくなる。");
				render_.draw_text(0, 16*5, "Graphics Image Light Bilk IgIiIrliiljkffkL\n", true);
				render_.draw_text(0, 16*6, "012:;,.(i),[i],{i},{|}.(`)\n", true);
			}
		}

		if(render) {
			int16_t xc = 470 / 2;
			int16_t yc = 272 / 2;
			render_.arc(xc + 0, yc + 100, xc, yc, xc + 0, yc - 100, RENDER::COLOR::White);
			render_.arc(xc + 0, yc - 100, xc, yc, xc + 0, yc + 100, RENDER::COLOR::Green);

			auto p0 = imath::circle::angle_to_position( 0.25f - 1.0f / 16.0f, 50);
			auto p1 = imath::circle::angle_to_position(-0.25f + 1.0f / 16.0f, 50);
			render_.arc(xc + p0.x, yc + p0.y, xc, yc, xc + p1.x, yc + p1.y, RENDER::COLOR::Red);

//			imath::circle cir;
//			cir.func_test();

#if 0
			render_.round_frame(20, 20, 400, 200, 16, RENDER::COLOR::Red);
//			render_.round_frame(20 + 5, 20 + 5, 400 - 10, 200 - 10, 16 - 5, RENDER::COLOR::Green);

			render_.round_frame(20 + 5, 20 + 5, 400 - 10, 200 - 10, 16 - 5, RENDER::COLOR::Red);
			render_.round_box(20 + 5, 20 + 5, 400 - 10, 200 - 10, 16 - 5, RENDER::COLOR::Green);

			render_.circle(480/2, 272/2, 100, RENDER::COLOR::Blue);
			render_.fill_circle(480/2, 272/2, 100, RENDER::COLOR::Aqua);

			render_.fill_box(5, 5, 4, 4, RENDER::COLOR::White);
			render_.frame(4, 4, 6, 6, RENDER::COLOR::Gray);
#endif
			render = false;
		}
#endif

		auto tnum = ft5206_.get_touch_num();
		if(tnum == 3) {
//			render_.clear(RENDER::COLOR::Black);
		}
		if(tnum > 0) {
			const auto& npos = ft5206_.get_touch_pos(0);
//			if(npos.event != FT5206::EVENT::CONTACT) {
//				pos.event = FT5206::EVENT::NONE;
//			}
//			if(pos.event == FT5206::EVENT::CONTACT) {
//				render_.line(pos.x, pos.y, npos.x, npos.y, RENDER::COLOR::White);
//			}
//			pos = npos;
		}

		render_task &= 3;
		switch(render_task) {
		case 0:
			render_.clear(DEF_COLOR::Black);
			break;
		case 1:
			render_.clear(DEF_COLOR::Black);
			render_.set_fore_color(DEF_COLOR::White);
			render_.line(vtx::spos(0, 0), vtx::spos(480, 272));
			render_.set_fore_color(DEF_COLOR::Green);
			render_.circle(vtx::spos(480/2, 272/2), 120, 10);

			render_.set_fore_color(DEF_COLOR::Purple);
			render_.fill_box(vtx::srect(100, 50, 90, 45));
			break;
       		case 2:
			render_.clear(DEF_COLOR::Black);
			render_.set_fore_color(DEF_COLOR::Blue);
			render_.circle(vtx::spos(480/2, 272/2), rad, 0);
			break;
		case 3:
			render_.clear(DEF_COLOR::Black);
			render_.set_fore_color(DEF_COLOR::Blue);
			render_.set_back_color(DEF_COLOR::White);
			render_.draw_text(vtx::spos(10, 120), "Asdfghjkl");
			render_.draw_text(vtx::spos(10, 136), "美しい漢字");
			render_.set_fore_color(DEF_COLOR::Fuchsi);
			render_.round_frame(vtx::srect(10, 10, 100, 50), 15);
			render_.set_fore_color(DEF_COLOR::Aqua);
			render_.round_box(vtx::srect(10, 10, 100, 50), 15);
			dialog_.modal(vtx::spos(200, 60), "モーダル・ダイアログ");
			break;

		default:
			break;
		}

		++rad;
		if(rad >= 256) rad = 10;

		command_();

		{  // SW2 の検出
			auto f = SW2::P();
			if(sw2 && !f) {
				utils::format("SW2: Positive\n");
				++render_task;
			}
			if(!sw2 && f) {
				utils::format("SW2: Negative\n");
			}
			sw2 = f;
		}

		++n;
		if(n >= 30) {
			n = 0;
		}
		if(n < 10) {
			LED::P = 0;
		} else {
			LED::P = 1;
		}
	}
}
