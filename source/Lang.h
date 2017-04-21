#ifndef Lang_h__
#define Lang_h__

enum
{
	MSG_PLUGIN_NAME,
	MSG_PLUGIN_CONFIG_NAME,

	// Action menu
	MSG_MENU_GENERATE,
	MSG_MENU_COMPARE,
	MSG_MENU_VALIDATE,
	MSG_MENU_VALIDATE_WITH_PARAMS,
	MSG_MENU_COMPARE_CLIP,
	
	// Configuration
	MSG_CONFIG_TITLE,
	MSG_CONFIG_PREFIX,
	MSG_CONFIG_DEFAULT_ALGO,
	MSG_CONFIG_CONFIRM_ABORT,
	MSG_CONFIG_CLEAR_SELECTION,
	MSG_CONFIG_AUTOEXT,
	MSG_CONFIG_UPPERCASE,
	MSG_CONFIG_REMEMBER_LAST_ALGO,
	MSG_CONFIG_DEFAULT_CP,
	MSG_CONFIG_DEFAULT_OUTPUT,
	MSG_CONFIG_OUTPUT_SINGLE_FILE,
	MSG_CONFIG_OUTPUT_SEPARATE_FILE,
	MSG_CONFIG_OUTPUT_DISPLAY,
	
	// Buttons
	MSG_BTN_OK,
	MSG_BTN_CANCEL,
	MSG_BTN_RUN,
	MSG_BTN_CLOSE,
	MSG_BTN_CLIPBOARD,
	MSG_BTN_RETRY,
	MSG_BTN_SKIP,
	MSG_BTN_SKIPALL,
	MSG_BTN_FILTER,

	// Dialogs
	MSG_DLG_ERROR,
	MSG_DLG_CONFIRM,
	MSG_DLG_NOTVALIDLIST,
	MSG_DLG_PROCESSING,
	MSG_DLG_PREPARE_LIST,
	MSG_DLG_OVERWRITE_FILE,
	MSG_DLG_OVERWRITE_FILE_TEXT,
	MSG_DLG_NO_FILES_SELECTED,
	MSG_DLG_NOFILES_TITLE,
	MSG_DLG_NOFILES_TEXT,
	MSG_DLG_GENERATING,
	MSG_DLG_ASK_ABORT,
	MSG_DLG_VALIDATION_COMPLETE,
	MSG_DLG_CALC_COMPLETE,
	MSG_DLG_MISMATCHED_FILES,
	MSG_DLG_MISSING_FILES,
	MSG_DLG_COMPARE,
	MSG_DLG_FILE_ERROR,
	MSG_DLG_FILE_PANEL_REQUIRED,
	MSG_DLG_NO_MISMATCHES,
	MSG_DLG_NUM_SKIPPED,
	MSG_DLG_NO_COMPARE_SELF,
	MSG_DLG_LOOKS_NO_HASH,
	MSG_DLG_CLIP_ERROR,
	MSG_DLG_FILE_CLIP_MATCH,
	MSG_DLG_FILE_CLIP_MISMATCH,
	MSG_DLG_PROGRESS,
	MSG_DLG_CANT_SAVE_HASHLIST,
	MSG_DLG_USE_FILTER,

	// Generation
	MSG_GEN_TITLE,
	MSG_GEN_ALGO,
	MSG_GEN_RECURSE,
	MSG_GEN_ABSPATH,
	MSG_GEN_TARGET,
	MSG_GEN_TO_FILE,
	MSG_GEN_TO_SEPARATE,
	MSG_GEN_TO_SCREEN,
	MSG_GEN_CODEPAGE,

	// Algo list
	MSG_ALGO_CRC,
	MSG_ALGO_MD5,
	MSG_ALGO_SHA1,
	MSG_ALGO_SHA256,
	MSG_ALGO_SHA512,
	MSG_ALGO_SHA3_512,
	MSG_ALGO_WHIRLPOOL,
};

#endif // Lang_h__