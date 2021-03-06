#pragma once
//=====================================================================//
/*!	@file
	@brief	ファイル選択ユーティリティー
    @author 平松邦仁 (hira@rvf-rc45.net)
	@copyright	Copyright (C) 2018, 2019 Kunihito Hiramatsu @n
				Released under the MIT license @n
				https://github.com/hirakuni45/RX/blob/master/LICENSE
*/
//=====================================================================//
#include "common/file_io.hpp"
#include "common/fixed_stack.hpp"

namespace gui {

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	/*!
		@brief	ファイラー制御型
	*/
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	enum class filer_ctrl {
		MOUNT,		///< ＳＤカードのマウント状態
		OPEN,		///< ファイラーを起動
		UP,			///< スクロール上
		DOWN,		///< スクロール下
		BACK,		///< ディレクトリーを戻る
		SELECT,		///< 選択
	};


	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	/*!
		@brief	制御データ構築
		@param[in]	pos		ファイラー制御型
		@param[in]	ctrl	制御データ（参照）
	*/
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	static inline void set(filer_ctrl pos, uint32_t& ctrl) noexcept
	{
		ctrl |= 1 << static_cast<uint8_t>(pos);
	}


	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	/*!
		@brief	ファイラー・クラス
		@param[in]	RDR	render クラス型（描画）
	*/
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	template <class RDR>
	class filer {

		static const int16_t SPC = 2;									///< 文字間隙間
		static const int16_t FLN = RDR::font_type::height + SPC;		///< 行幅
		static const int16_t SCN = (RDR::glc_type::height - SPC) / FLN;	///< 行数

		typedef graphics::def_color DEF_COLOR;

		RDR&		rdr_;

		typedef utils::dir_list DLIST;
		DLIST		dlist_;

		uint32_t	ctrl_;
		bool		open_;

		struct rdr_st {
			int16_t		vofs_;
			int16_t		vpos_;
			int16_t		hmax_;
			int16_t		sel_pos_;
			uint16_t	num_;
			int16_t		match_;

			rdr_st() noexcept : vofs_(0), vpos_(0), hmax_(0),
				sel_pos_(0), num_(0), match_(-1)
			{ }
		};
		rdr_st	rdr_st_;

		struct pos_t {
			int16_t		vofs_;
			int16_t		sel_pos_;
			pos_t(int16_t vofs = 0, int16_t sel_pos = 0) noexcept :
				vofs_(vofs), sel_pos_(sel_pos) { }
		};
		typedef utils::fixed_stack<pos_t, 16> POS_STACK;
		POS_STACK		pos_stack_;

		bool			touch_lvl_;
		bool			touch_pos_;
		bool			touch_neg_;
		uint8_t			touch_num_;
		int16_t			touch_x_;
		int16_t			touch_y_;
		int16_t			touch_org_x_;
		int16_t			touch_org_y_;
		int16_t			touch_end_x_;
		int16_t			touch_end_y_;

		uint8_t			back_num_;

		static uint32_t ctrl_mask_(filer_ctrl ctrl) noexcept
		{
			return 1 << static_cast<uint8_t>(ctrl);
		}


		void dir_draw_func_(const char* name, const FILINFO* fi, bool dir, void* opt) noexcept
		{
			rdr_st& t = *static_cast<rdr_st*>(opt);
			bool draw = false;
			if(t.match_ < 0) {
				draw = true;
			} else if(t.num_ == static_cast<uint16_t>(t.match_)) {
				draw = true;
			}
			if(draw && t.vpos_ >= 0 && t.vpos_ < RDR::glc_type::height) {
				rdr_.set_fore_color(DEF_COLOR::Black);
				rdr_.fill_box(vtx::srect(SPC, t.vpos_,
					RDR::glc_type::width - SPC * 2, RDR::font_type::height));
				rdr_.set_fore_color(DEF_COLOR::White);
				if(dir) rdr_.draw_font(vtx::spos(SPC, t.vpos_), '/');
				if(dir) {
					rdr_.set_fore_color(DEF_COLOR::Blue);
				} else {
					rdr_.set_fore_color(DEF_COLOR::White);
				}
				auto w = rdr_.draw_text(vtx::spos(SPC + 8, t.vpos_), name);
				if(t.hmax_ < w) t.hmax_ = w;
			}
			t.vpos_ += FLN;
			++t.num_;
		}


		void start_dir_draw_(int16_t match = -1) noexcept
		{
			rdr_st_.vpos_ = rdr_st_.vofs_ + 2;
			rdr_st_.num_ = 0;
			rdr_st_.match_ = match;
			char tmp[FF_MAX_LFN + 1];
			if(utils::file_io::pwd(tmp, sizeof(tmp))) {
				dlist_.start(tmp);
			}
		}


		void draw_sel_frame_(int16_t pos)
		{
			int16_t h = RDR::font_type::height + 2;
			int16_t y = pos * h;
			rdr_.frame(vtx::srect(0, y, rdr_st_.hmax_ + 3, h + 1));
		}


		void scan_dir_(bool back)
		{
			if(back) {
				if(pos_stack_.empty()) {
					rdr_st_.vofs_ = 0;
					rdr_st_.sel_pos_ = 0;
				} else {
					const auto& t = pos_stack_.pop();
					rdr_st_.vofs_ = t.vofs_;
					rdr_st_.sel_pos_ = t.sel_pos_;
				}
			} else {
				rdr_st_.vofs_ = 0;
				rdr_st_.sel_pos_ = 0;
			}
			start_dir_draw_();
		}

	public:
		//-----------------------------------------------------------------//
		/*!
			@brief	コンストラクター
			@param[in]	rdr		レンダリング・インスタンス
		*/
		//-----------------------------------------------------------------//
		filer(RDR& rdr) noexcept : rdr_(rdr), dlist_(),
			ctrl_(0), open_(false), rdr_st_(),
			touch_lvl_(false), touch_pos_(false), touch_neg_(false), touch_num_(0),
			touch_x_(0), touch_y_(0),
			touch_org_x_(0), touch_org_y_(0), touch_end_x_(0), touch_end_y_(0),
			back_num_(0)
		{ }


		//-----------------------------------------------------------------//
		/*!
			@brief	スクリーン・タッチ位置設定 @n
					※タッチ情報は毎フレーム設定する事。
			@param[in]	num		タッチ数
			@param[in]	x		タッチＸ
			@param[in]	y		タッチＹ
		*/
		//-----------------------------------------------------------------//
		void set_touch(uint8_t num, int16_t x, int16_t y) noexcept
		{
			touch_num_ = num;
			bool lvl = num > 0;
			touch_pos_ = !touch_lvl_ &  lvl;
			touch_neg_ =  touch_lvl_ & !lvl;
			touch_lvl_ = lvl;
			touch_x_ = x;
			touch_y_ = y;
			if(touch_pos_) {
				touch_org_x_ = touch_x_;
				touch_org_y_ = touch_y_;
			}
			if(touch_neg_) {
				touch_end_x_ = touch_x_;
				touch_end_y_ = touch_y_;
			}
		}


		//-----------------------------------------------------------------//
		/*!
			@brief	アップデート（毎フレーム呼ぶ）
			@param[in]	ctrl	ファイラー制御
			@param[in]	dst		選択パス受け取り先
			@param[in]	dstlen	選択パス最大サイズ
			@return ファイルが選択された場合「true」
		*/
		//-----------------------------------------------------------------//
		bool update(uint32_t ctrl, char* dst, uint32_t dstlen) noexcept
		{
			uint32_t ptrg = ~ctrl_ &  ctrl;
			uint32_t ntrg =  ctrl_ & ~ctrl;
			ctrl_ = ctrl;

			dlist_.service(10,
				[&](const char* name, const FILINFO* fi, bool dir, void* opt) {
				dir_draw_func_(name, fi, dir, opt); }, true, &rdr_st_);

			if((ntrg & ctrl_mask_(filer_ctrl::MOUNT))) {  // SD カードのマウント状態
				open_ = false;
				pos_stack_.clear();
				rdr_.clear(DEF_COLOR::Black);
			}
			if((ctrl & ctrl_mask_(filer_ctrl::MOUNT)) == 0) {
				return false;
			}

			if((ptrg & ctrl_mask_(filer_ctrl::OPEN)) != 0 || (back_num_ == 3 && touch_num_ < 3)) {
				open_ = !open_;
				rdr_.clear(DEF_COLOR::Black);
				if(open_) {
					rdr_.at_font().at_kfont().flush_cash();
					scan_dir_(false);
				}
			}
			back_num_ = touch_num_;

			if(!open_) {
				return false;
			}

			{  // ドラッグでフォーカス移動
				if(touch_neg_) {
					auto dx = touch_x_ - touch_org_x_;
					if(dx >= FLN * 2) {
						ptrg |= ctrl_mask_(filer_ctrl::SELECT);
						touch_org_x_ = touch_x_;
					} else if(dx <= -FLN * 2) {
						ptrg |= ctrl_mask_(filer_ctrl::BACK);
						touch_org_x_ = touch_x_;
					}
				}
				if(touch_lvl_) {
					auto dy = touch_y_ - touch_org_y_;
					if(dy >= FLN) {
						ptrg |= ctrl_mask_(filer_ctrl::DOWN);
						touch_org_y_ = touch_y_;
					} else if(dy <= -FLN) {
						ptrg |= ctrl_mask_(filer_ctrl::UP);
						touch_org_y_ = touch_y_;
					}
				}
			}

			{
				if(dlist_.probe()) return false;
				if(rdr_st_.num_ < static_cast<int16_t>(dlist_.get_total())) {
					return false;
				}
			}

			// 選択フレームの描画
			rdr_.set_fore_color(DEF_COLOR::White);
			draw_sel_frame_(rdr_st_.sel_pos_);
			int16_t pos = rdr_st_.sel_pos_;
			if(ptrg & ctrl_mask_(filer_ctrl::UP)) {
				pos--;
			}
			if(ptrg & ctrl_mask_(filer_ctrl::DOWN)) {
				++pos;
			}
			int16_t vofs = rdr_st_.vofs_;
			int16_t scn = SCN;
			if(rdr_st_.num_ < scn) scn = rdr_st_.num_; 
			if(pos < 0) {
				pos = 0;
				vofs += FLN;
			} else if(pos >= scn) {
				pos = scn - 1;
				vofs -= FLN;
			}
			int16_t lim = 0;
			if(rdr_st_.num_ > scn) {
				lim = -(rdr_st_.num_ - scn) * FLN;
			}
			if(vofs > 0) {
				vofs = 0;
			} else if(vofs < lim) {
				vofs = lim;
			}
			if(vofs != rdr_st_.vofs_) {
				rdr_.set_fore_color(DEF_COLOR::Black);
				draw_sel_frame_(rdr_st_.sel_pos_);  // delete frame
				int16_t match = -1;
				if(vofs < rdr_st_.vofs_) {  // down
					rdr_.scroll(FLN);
					match = -vofs / FLN + (RDR::glc_type::height / FLN) - 1;
				} else if(vofs > rdr_st_.vofs_) {  // up
					rdr_.scroll(-FLN);
					match = -vofs / FLN;
				}
				rdr_st_.vofs_ = vofs;
				start_dir_draw_(match);
			}
			
			if(pos != rdr_st_.sel_pos_) {
				rdr_.set_fore_color(DEF_COLOR::Black);
				draw_sel_frame_(rdr_st_.sel_pos_);
				rdr_st_.sel_pos_ = pos;
			}

			if(ptrg & ctrl_mask_(filer_ctrl::SELECT)) {
				uint32_t idx = rdr_st_.sel_pos_ - rdr_st_.vofs_ / FLN;
				char root[FF_MAX_LFN + 1];
				if(!utils::file_io::pwd(root, sizeof(root))) {
					return false;
				}
				if(!utils::file_io::get_dir_path(root, idx, dst, dstlen)) {
					return false;
				}
				uint32_t l = strlen(dst);
				if(dst[l - 1] == '/') {  // directory ?
					pos_stack_.push(pos_t(rdr_st_.vofs_, rdr_st_.sel_pos_));
					dst[l - 1] = 0;
					utils::file_io::cd(dst);
					rdr_.clear(DEF_COLOR::Black);
					scan_dir_(false);
				} else {
					rdr_.clear(DEF_COLOR::Black);
					open_ = false;
					return true;
				}
			}

			if(ptrg & ctrl_mask_(filer_ctrl::BACK)) {
				if(!pos_stack_.empty()) {
					utils::file_io::cd("..");
					rdr_.clear(DEF_COLOR::Black);
					scan_dir_(true);
				}
			}

			return false;
		}
	};
}
