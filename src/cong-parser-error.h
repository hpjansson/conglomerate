/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

#ifndef __CONG_PARSER_ERROR_H
#define __CONG_PARSER_ERROR_H

G_BEGIN_DECLS

/* Towards improved error handling: */
typedef enum
{
	CONG_ISSUE_ERROR,
	CONG_ISSUE_WARNING
} CongIssueType;

typedef struct CongParserIssue
{
	CongIssueType type;
	gchar *filename;
	int linenum;
	gchar *description;
} CongParserIssue;

typedef struct CongParserResult
{
	const char *buffer;
	GnomeVFSFileSize size;
	GSList *issues; /* list of CongParserIssues */

	const gchar *string_uri;
	GtkWindow *parent_window;
} CongParserResult;

xmlDocPtr 
cong_ui_parse_buffer (const char* buffer, 
		      GnomeVFSFileSize size, 
		      const gchar *string_uri,
		      GtkWindow *parent_window);

void cong_parser_result_add_issue(CongParserResult *result, CongIssueType type, int linenum, gchar *description);

GtkDialog *cong_parser_result_dialog_new(CongParserResult *parser_result);

GtkDialog*
cong_error_dialog_new_file_open_failed_from_parser_error (const gchar* string_uri, 
							  CongParserResult *parser_result);

xmlParserCtxtPtr
cong_parse_from_filename (const gchar* string_uri,
			  CongParserResult *parser_result);

xmlParserCtxtPtr
cong_parse_from_memory (const char* buffer, 
			GnomeVFSFileSize size,
			CongParserResult *parser_result);

G_END_DECLS

#endif
