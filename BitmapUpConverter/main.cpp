#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <crtdbg.h>
#include <conio.h>
#include <cstdint>
#include <string>
#include "image_up_converter.h"

/////////////////////// プロトタイプ宣言 //////////////////////
static int file_setting( std::string& , std::string& , int * );
///////////////////////////////////////////////////////////////

void main(){

	while( true ){

		// 読み込むファイルと書き込むファイル
		std::string import_file_name = "color_file";
		std::string export_file_name = "convert_file";

		// 読み込み用のファイル名、書き込み用のファイル名を設定する
		int file_change_flg = 0;
		int file_type = file_setting( import_file_name , export_file_name , &file_change_flg );
		
		// ファイルの情報を読み込む
		color_data_origin *color_data_origin = image_data_create( import_file_name.c_str() , file_change_flg );	
		if( color_data_origin == NULL ){ printf("ファイルオープンエラー\n"); break; }	// ファイルオープンエラーチェック

		// ビットマップ画像のみアップコンバートを行う
		if( file_type == BMP_MODE ){
			int select_bit_type;
			printf("何ビットにアップコンバートしますか？\n0:32bit\n1:24bit\n2:16bit\n3:8bit\n4:アップコンバートしない\n");
			scanf_s( "%d" , &select_bit_type );
			puts("");
			image_data_up_convert( color_data_origin , select_bit_type );
		}
		
		// 画像データを変換する
		int select_transfer_type;
		printf("1～3の数字を入力してください\n1:グレースケール\n2:セピア\n3:ネガポジ\n");
		scanf_s( "%d" , &select_transfer_type );
		puts("");
		int error_code = image_data_transfer( select_transfer_type , color_data_origin );
		if( error_code == ERR_CONVERT ){
			printf("1～3で入力してください");
			_getch();
		}else{ 
			// ファイルへの書き込み処理
			image_data_write( color_data_origin , export_file_name.c_str() , file_type , file_change_flg ); 
		}

		// 解放処理
		image_data_release( color_data_origin );
	}
	
	_CrtDumpMemoryLeaks();
}

//　ファイルの種類、名前を決定する関数
static int file_setting( std::string& import_file_name , std::string& export_file_name , int *file_change_flg ){
	
	while( 1 ){

		// 各選択処理を行う
		int select_mode = 0;
		system("cls");
		printf("bmpファイルとpngファイル どちらを使用しますか？\n1:bmp 2:png\n");
		scanf_s( "%d" , &select_mode );
		puts("");
		
		// ファイルの拡張子の変更を行うかの選択
		if( select_mode == 1 || select_mode == 2 ){ 
			( select_mode == 1 ) ? printf("ファイルの種類をpngに変更しますか？ 1:はい　1以外:いいえ\n") : printf("ファイルの種類をbmpに変更しますか？ 1:はい　1以外:いいえ\n");
		}else{ 
			printf("1～2で入力してください\n"); 
			continue;
		}
		int change_flg = 0;
		scanf_s( "%d" , &change_flg );
		puts("");
		if( change_flg == 1 ){ *file_change_flg = 1; }

		if( select_mode == BMP_MODE ){
			// bmpファイルを使用する場合
			printf("何ビットのファイルを使用しますか？\n1:24bit\n2:16bit\n3:8bit\n4:4bit\n");
			scanf_s( "%d" , &select_mode );
			puts("");
		
			// 変換元のファイルのビット数を選択
			if( select_mode == SELECT_24_BIT ){ import_file_name += "_24bit"; }
			else if( select_mode == SELECT_16_BIT ){ import_file_name += "_16bit"; }
			else if( select_mode == SELECT_8_BIT ){ import_file_name += "_8bit"; }
			else if( select_mode == SELECT_4_BIT ){ import_file_name += "_4bit"; }
			else{ printf("1～4で入力してください\n"); continue; }

			// 出力ファイルの拡張子を変更するかどうかの処理
			import_file_name += ".bmp";
			if( change_flg == 1 ){
				export_file_name += ".png";
				change_flg = PNG_MODE;
			}else{
				export_file_name += ".bmp";
				change_flg = BMP_MODE;
			}
			return change_flg;

		}else if( select_mode == PNG_MODE ){
			// png画像を使用する場合
			import_file_name += ".png";
			if( change_flg == 1 ){
				export_file_name += ".bmp";
				change_flg = BMP_MODE;
			}else{
				export_file_name += ".png";
				change_flg = PNG_MODE;
			}
			return change_flg;

		}else{
			printf("1～2で入力してください\n");
			continue;
		}
	}
}