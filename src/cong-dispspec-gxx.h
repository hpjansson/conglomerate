/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * cong-dispspec-gxx.h
 * 
 * This file will be included multiple times, with different results.
 *
 * Returns:
 */

GXX_STRUCT_BEGIN_ELEMENT("format", CongSerialisationFormat, serialisation_format)
     GXX_STRUCT_ATTRIBUTE_STRING ("extension", extension, TRUE, "")
GXX_STRUCT_END_ELEMENT()

GXX_STRUCT_BEGIN_ELEMENT("external-document-model", CongExternalDocumentModel, external_document_model)
     GXX_STRUCT_ATTRIBUTE_ENUM ("type", model_type, TRUE, CONG_DOCUMENT_MODE_TYPE_DTD, document_model_enum_mapping)
     GXX_STRUCT_ATTRIBUTE_STRING ("public-id", public_id, FALSE, "")
     GXX_STRUCT_ATTRIBUTE_STRING ("system-id", system_id, FALSE, "")
GXX_STRUCT_END_ELEMENT()



