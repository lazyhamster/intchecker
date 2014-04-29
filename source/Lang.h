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
	
	// Configuration
	MSG_CONFIG_TITLE,
	MSG_CONFIG_PREFIX,
	MSG_CONFIG_DEFAULT_ALGO,
	MSG_CONFIG_CONFIRM_ABORT,
	MSG_CONFIG_CLEAR_SELECTION,
	MSG_CONFIG_AUTOEXT,

	// Buttons
	MSG_BTN_OK,
	MSG_BTN_CANCEL,
	MSG_BTN_RUN,
	MSG_BTN_CLOSE,
	MSG_BTN_CLIPBOARD,
	MSG_BTN_RETRY,
	MSG_BTN_SKIP,

	// Dialogs
	MSG_DLG_ERROR,
	MSG_DLG_CONFIRM,
	MSG_DLG_NOTVALIDLIST,
	MSG_DLG_PROCESSING,
	MSG_DLG_PREPARE_LIST,
	MSG_DLG_OVERWRITE_FILE,
	MSG_DLG_OVERWRITE_FILE_TEXT,
	MSG_DLG_INVALID_PANEL,
	MSG_DLG_NOFILES_TITLE,
	MSG_DLG_NOFILES_TEXT,
	MSG_DLG_GENERATING,
	MSG_DLG_ASK_ABORT,
	MSG_DLG_VALIDATION_COMPLETE,
	MSG_DLG_CALC_COMPLETE,

	// Generation
	MSG_GEN_TITLE,
	MSG_GEN_ALGO,
	MSG_GEN_RECURSE,
	MSG_GEN_TARGET,
	MSG_GEN_TO_FILE,
	MSG_GEN_TO_SEPARATE,
	MSG_GEN_TO_SCREEN,
};

#endif // Lang_h__