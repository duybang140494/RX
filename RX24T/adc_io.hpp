#pragma once
//=====================================================================//
/*!	@file
	@brief	RX24T グループ A/D 制御 @n
			Copyright 2016,2017 Kunihito Hiramatsu
	@author	平松邦仁 (hira@rvf-rc45.net)
*/
//=====================================================================//
#include "RX24T/icu_mgr.hpp"
#include "RX24T/port_map.hpp"
#include "RX24T/power_cfg.hpp"
#include "common/vect.h"

/// F_PCKD は変換パラメーター計算で必要で、設定が無いとエラーにします。
#ifndef F_PCKD
#  error "adc_io.hpp requires F_PCKD to be defined"
#endif

namespace device {

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	/*!
		@brief  A/D 制御クラス
		@param[in]	ADCU	A/D チャネル・ユニット
		@param[in]	TASK	割り込みタスク
	*/
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	template <class ADCU, class TASK>
	class adc_io {
	public:

		typedef ADCU value_type;

		//-----------------------------------------------------------------//
		/*!
			@brief	変換モード
		 */
		//-----------------------------------------------------------------//
		enum class cnv_type : uint8_t {
			
		};


	private:
		static TASK task_;

		static INTERRUPT_FUNC void adi_task_()
		{
			task_();
		}

		uint8_t	level_;

		static inline void sleep_() { asm("nop"); }

	public:
		//-----------------------------------------------------------------//
		/*!
			@brief	コンストラクター
		 */
		//-----------------------------------------------------------------//
		adc_io() : level_(0) { }


		//-----------------------------------------------------------------//
		/*!
			@brief	スタート
			@param[in]	ana		アナログ入力
			@param[in]	level	割り込みレベル、０の場合はポーリング
			@return 成功なら「true」
		 */
		//-----------------------------------------------------------------//
		bool start(typename ADCU::analog ana, uint8_t level = 0)
		{
			level_ = level;

			// 基本変換時間（１マイクロ秒）＋マージン
			uint32_t n = F_PCKD / 1000000 + 10;
			if(n > 255) return false;

			power_cfg::turn(ADCU::get_peripheral());
			ADCU::enable(ana);
			ADCU::ADANSA.set(ana);
			ADCU::ADSSTR.set(ana, n);

			return true;
		}


		//-----------------------------------------------------------------//
		/*!
			@brief	スキャン開始
		 */
		//-----------------------------------------------------------------//
		void scan() {
			if(level_) {
				set_interrupt_task(adi_task_, static_cast<uint32_t>(ADCU::get_vec()));
				icu_mgr::set_level(ADCU::get_peripheral(), level_);
				ADCU::ADCSR = ADCU::ADCSR.ADST.b() | ADCU::ADCSR.ADIE.b();
			} else {
				ADCU::ADCSR = ADCU::ADCSR.ADST.b();
			}
		}


		//-----------------------------------------------------------------//
		/*!
			@brief	変換状態の取得
			@return 変換中なら「true」
		 */
		//-----------------------------------------------------------------//
		bool get_state() const {
			return ADCU::ADCSR.ADST();
		}


		//-----------------------------------------------------------------//
		/*!
			@brief	読み込み同期
		 */
		//-----------------------------------------------------------------//
		void sync() const {
			while(ADCU::ADCSR.ADST() != 0) sleep_();
		}


		//-----------------------------------------------------------------//
		/*!
			@brief	A/D 変換結果を取得
			@param[in]	an	変換チャネル
			@return 変換結果（上位１０ビットが有効な値）
		 */
		//-----------------------------------------------------------------//
		uint16_t get(typename ADCU::analog an) const {
			return ADCU::ADDR(an);
		}
	};

	template <class ADCU, class TASK>
	TASK adc_io<ADCU, TASK>::task_;
}
