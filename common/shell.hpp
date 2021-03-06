#pragma once
//=====================================================================//
/*!	@file
	@brief	シェル、ＳＤコマンド解析、実行 @n
			・dir [xxx] @n
			・pwd @n
			・cd [xxx]
    @author 平松邦仁 (hira@rvf-rc45.net)
	@copyright	Copyright (C) 2019 Kunihito Hiramatsu @n
				Released under the MIT license @n
				https://github.com/hirakuni45/RX/blob/master/LICENSE
*/
//=====================================================================//
#include "common/file_io.hpp"
#include "common/format.hpp"

namespace utils {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    /*!
        @brief  command class
		@param[in]	CMD		コマンド入力
    */
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
	template <class CMD>
	class shell {

		CMD&	cmd_;

		bool	sdc_state_;

	public:
        //-----------------------------------------------------------------//
        /*!
            @brief  コンストラクター
        */
        //-----------------------------------------------------------------//
		shell(CMD& cmd) : cmd_(cmd), sdc_state_(true)
		{ }


        //-----------------------------------------------------------------//
        /*!
            @brief  ＳＤ操作の完了を取得
			@return ＳＤ操作が正常なら「true」
        */
        //-----------------------------------------------------------------//
		bool get_status() const { return sdc_state_; }


        //-----------------------------------------------------------------//
        /*!
            @brief  基本ＳＤ操作コマンド解析
			@return ＳＤ操作にマッチしたら「true」
        */
        //-----------------------------------------------------------------//
		bool analize() noexcept
		{
            auto cmdn = cmd_.get_words();
            if(cmdn >= 1) {
                if(cmd_.cmp_word(0, "dir")) {  // dir [xxx]
					char tmp[FF_MAX_LFN + 1];
					if(cmdn == 1) {
						if(!utils::file_io::pwd(tmp, sizeof(tmp))) {
							sdc_state_ = false;
						} else {
							sdc_state_ = utils::file_io::dir(tmp);
						}
					} else {
						cmd_.get_word(1, tmp, sizeof(tmp));
						sdc_state_ = utils::file_io::dir(tmp);
					}
				} else if(cmd_.cmp_word(0, "pwd")) {  // pwd
					char tmp[FF_MAX_LFN + 1];
					if(!utils::file_io::pwd(tmp, sizeof(tmp))) {
						sdc_state_ = false;
					} else {
						utils::format("%s\n") % tmp;
					}
				} else if(cmd_.cmp_word(0, "cd")) {  // cd [xxx]
					char tmp[FF_MAX_LFN + 1];
					if(cmdn == 1) {
						strcpy(tmp, "/");
					} else {
						cmd_.get_word(1, tmp, sizeof(tmp));
					}
					sdc_state_ = utils::file_io::cd(tmp);
				} else {
					return false;
				}
			}
			return true;
		}


        //-----------------------------------------------------------------//
        /*!
            @brief  HELP 表示
        */
        //-----------------------------------------------------------------//
		void help() const {
			utils::format("    dir [xxx]       list current directory\n");
			utils::format("    pwd             current directory path\n");
			utils::format("    cd [xxx]        change current directory\n");
///						   ---+---+---+---+---+
		}
	};
}
