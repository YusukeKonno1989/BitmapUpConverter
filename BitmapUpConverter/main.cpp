#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <crtdbg.h>
#include <conio.h>
#include <cstdint>
#include <string>
#include "image_up_converter.h"

/////////////////////// �v���g�^�C�v�錾 //////////////////////
static int file_setting( std::string& , std::string& , int * );
///////////////////////////////////////////////////////////////

void main(){

	while( true ){

		// �ǂݍ��ރt�@�C���Ə������ރt�@�C��
		std::string import_file_name = "color_file";
		std::string export_file_name = "convert_file";

		// �ǂݍ��ݗp�̃t�@�C�����A�������ݗp�̃t�@�C������ݒ肷��
		int file_change_flg = 0;
		int file_type = file_setting( import_file_name , export_file_name , &file_change_flg );
		
		// �t�@�C���̏���ǂݍ���
		color_data_origin *color_data_origin = image_data_create( import_file_name.c_str() , file_change_flg );	
		if( color_data_origin == NULL ){ printf("�t�@�C���I�[�v���G���[\n"); break; }	// �t�@�C���I�[�v���G���[�`�F�b�N

		// �r�b�g�}�b�v�摜�̂݃A�b�v�R���o�[�g���s��
		if( file_type == BMP_MODE ){
			int select_bit_type;
			printf("���r�b�g�ɃA�b�v�R���o�[�g���܂����H\n0:32bit\n1:24bit\n2:16bit\n3:8bit\n4:�A�b�v�R���o�[�g���Ȃ�\n");
			scanf_s( "%d" , &select_bit_type );
			puts("");
			image_data_up_convert( color_data_origin , select_bit_type );
		}
		
		// �摜�f�[�^��ϊ�����
		int select_transfer_type;
		printf("1�`3�̐�������͂��Ă�������\n1:�O���[�X�P�[��\n2:�Z�s�A\n3:�l�K�|�W\n");
		scanf_s( "%d" , &select_transfer_type );
		puts("");
		int error_code = image_data_transfer( select_transfer_type , color_data_origin );
		if( error_code == ERR_CONVERT ){
			printf("1�`3�œ��͂��Ă�������");
			_getch();
		}else{ 
			// �t�@�C���ւ̏������ݏ���
			image_data_write( color_data_origin , export_file_name.c_str() , file_type , file_change_flg ); 
		}

		// �������
		image_data_release( color_data_origin );
	}
	
	_CrtDumpMemoryLeaks();
}

//�@�t�@�C���̎�ށA���O�����肷��֐�
static int file_setting( std::string& import_file_name , std::string& export_file_name , int *file_change_flg ){
	
	while( 1 ){

		// �e�I���������s��
		int select_mode = 0;
		system("cls");
		printf("bmp�t�@�C����png�t�@�C�� �ǂ�����g�p���܂����H\n1:bmp 2:png\n");
		scanf_s( "%d" , &select_mode );
		puts("");
		
		// �t�@�C���̊g���q�̕ύX���s�����̑I��
		if( select_mode == 1 || select_mode == 2 ){ 
			( select_mode == 1 ) ? printf("�t�@�C���̎�ނ�png�ɕύX���܂����H 1:�͂��@1�ȊO:������\n") : printf("�t�@�C���̎�ނ�bmp�ɕύX���܂����H 1:�͂��@1�ȊO:������\n");
		}else{ 
			printf("1�`2�œ��͂��Ă�������\n"); 
			continue;
		}
		int change_flg = 0;
		scanf_s( "%d" , &change_flg );
		puts("");
		if( change_flg == 1 ){ *file_change_flg = 1; }

		if( select_mode == BMP_MODE ){
			// bmp�t�@�C�����g�p����ꍇ
			printf("���r�b�g�̃t�@�C�����g�p���܂����H\n1:24bit\n2:16bit\n3:8bit\n4:4bit\n");
			scanf_s( "%d" , &select_mode );
			puts("");
		
			// �ϊ����̃t�@�C���̃r�b�g����I��
			if( select_mode == SELECT_24_BIT ){ import_file_name += "_24bit"; }
			else if( select_mode == SELECT_16_BIT ){ import_file_name += "_16bit"; }
			else if( select_mode == SELECT_8_BIT ){ import_file_name += "_8bit"; }
			else if( select_mode == SELECT_4_BIT ){ import_file_name += "_4bit"; }
			else{ printf("1�`4�œ��͂��Ă�������\n"); continue; }

			// �o�̓t�@�C���̊g���q��ύX���邩�ǂ����̏���
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
			// png�摜���g�p����ꍇ
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
			printf("1�`2�œ��͂��Ă�������\n");
			continue;
		}
	}
}