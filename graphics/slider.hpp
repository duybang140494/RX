#pragma once
//=====================================================================//
/*!	@file
	@brief	スライダー表示と制御
    @author 平松邦仁 (hira@rvf-rc45.net)
	@copyright	Copyright (C) 2019 Kunihito Hiramatsu @n
				Released under the MIT license @n
				https://github.com/hirakuni45/RX/blob/master/LICENSE
*/
//=====================================================================//
#include <functional>
#include "graphics/widget.hpp"

namespace gui {

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	/*!
		@brief	スライダー・クラス
	*/
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	struct slider : public widget {

		typedef slider value_type;

		typedef std::function<void(float ratio)> SELECT_FUNC_TYPE;

		static const int16_t round_radius = 9;
		static const int16_t frame_width  = 3;
		static const int16_t handle_size  = 18;

	private:

		SELECT_FUNC_TYPE	select_func_;
		vtx::spos			touch_org_;
		float				ratio_org_;
		float				ratio_;

	public:
		//-----------------------------------------------------------------//
		/*!
			@brief	コンストラクター
			@param[in]	loc		ロケーション
			@param[in]	str		ボタン文字列
		*/
		//-----------------------------------------------------------------//
		slider(const vtx::srect& loc = vtx::srect(0), const char* str = "") noexcept :
			widget(loc, str), select_func_(),
			touch_org_(0), ratio_org_(0.0f), ratio_(0.0f)
		{
			if(loc.size.x <= 0) {
			}
			if(loc.size.y <= 0) {
				at_location().size.y = handle_size;
			}
			insert_widget(this);
		}


		slider(const slider& th) = delete;
		slider& operator = (const slider& th) = delete;


		//-----------------------------------------------------------------//
		/*!
			@brief	デストラクタ
		*/
		//-----------------------------------------------------------------//
		virtual ~slider() { remove_widget(this); }


		//-----------------------------------------------------------------//
		/*!
			@brief	型整数を取得
			@return 型整数
		*/
		//-----------------------------------------------------------------//
		const char* get_name() const override { return "Slider"; }


		//-----------------------------------------------------------------//
		/*!
			@brief	ID を取得
			@return ID
		*/
		//-----------------------------------------------------------------//
		ID get_id() const override { return ID::SLIDER; }


		//-----------------------------------------------------------------//
		/*!
			@brief	初期化
		*/
		//-----------------------------------------------------------------//
		void init() override { }


		//-----------------------------------------------------------------//
		/*!
			@brief	選択推移
		*/
		//-----------------------------------------------------------------//
		void exec_select() override
		{
			if(get_touch_state().positive_) {
				touch_org_ = get_touch_state().position_;
				ratio_org_ = ratio_;
			} else if(get_touch_state().level_) {
				auto d = get_touch_state().position_ - touch_org_;
				float ref = 1.0f;
				float val = 0.0f;
				const auto& loc = get_location();
				if(loc.size.x > loc.size.y) {
					ref = static_cast<float>(loc.size.x - frame_width * 2 - loc.size.y);
					val = static_cast<float>(d.x);
				} else {
					ref = static_cast<float>(loc.size.y - frame_width * 2 - loc.size.x);
					val = static_cast<float>(d.y);
				}
				ratio_ = ratio_org_ + (val / ref);
				if(ratio_ < 0.0f) ratio_ = 0.0f;
				else if(ratio_ > 1.0f) ratio_ = 1.0f;
			}
			if(select_func_) {
				select_func_(ratio_);
			}
		}


		//-----------------------------------------------------------------//
		/*!
			@brief	スライダー・レシオの取得（移動量を正規化した値 0.0 to 1.0）
			@return	スライダー・レシオ
		*/
		//-----------------------------------------------------------------//
		float get_ratio() const noexcept { return ratio_; }


		//-----------------------------------------------------------------//
		/*!
			@brief	セレクト関数への参照
			@return	セレクト関数
		*/
		//-----------------------------------------------------------------//
		SELECT_FUNC_TYPE& at_select_func() noexcept { return select_func_; }


		//-----------------------------------------------------------------//
		/*!
			@brief	描画
			@param[in]	rdr		描画インスタンス
		*/
		//-----------------------------------------------------------------//
		template<class RDR>
		void draw(RDR& rdr) noexcept
		{
			auto r = get_location();
			rdr.set_fore_color(graphics::def_color::White);
			rdr.round_box(r, round_radius);
			if(get_touch_state().level_) {
				rdr.set_fore_color(graphics::def_color::Silver);
			} else {
				rdr.set_fore_color(graphics::def_color::Darkgray);
			}
			r.org  += frame_width;
			r.size -= frame_width * 2;
			rdr.round_box(r, round_radius - frame_width);

			const auto& size = get_location().size;
			auto cen = get_location().org;
			int16_t rad = 0;
			if(size.x > size.y) {
				rad = size.y / 2;
				--rad;
				cen.x += frame_width + rad;
				cen.y += rad;
				cen.x += (size.x - frame_width * 2 - size.y) * ratio_;
			} else {
				rad = size.x / 2;
				--rad;
				cen.y += frame_width + rad;
				cen.x += rad;
				cen.y += (size.y - frame_width * 2 - size.x) * ratio_;
			}
			rdr.set_fore_color(graphics::def_color::White);
			rdr.fill_circle(cen, rad);
		}
	};
}