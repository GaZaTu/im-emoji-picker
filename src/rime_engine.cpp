#include "rime_engine.hpp"
#include "rime_main.hpp"
#include <QMainWindow>
#include <string>

extern "C" {
typedef struct _IBusRimeEngine IBusRimeEngine;
typedef struct _IBusRimeEngineClass IBusRimeEngineClass;

struct _IBusRimeEngine {
  IBusEngine parent;

  // IBusLookupTable* table;
  // IBusPropList* props;

  QMainWindow* window;

  std::string text;
};

struct _IBusRimeEngineClass {
  IBusEngineClass parent;
};

static void ibus_rime_engine_class_init(IBusRimeEngineClass* klass);
static void ibus_rime_engine_init(IBusRimeEngine* rime_engine);
static void ibus_rime_engine_destroy(IBusRimeEngine* rime_engine);
static gboolean ibus_rime_engine_process_key_event(IBusEngine* engine, guint keyval, guint keycode, guint modifiers);
static void ibus_rime_engine_focus_in(IBusEngine* engine);
static void ibus_rime_engine_focus_out(IBusEngine* engine);
static void ibus_rime_engine_reset(IBusEngine* engine);
static void ibus_rime_engine_enable(IBusEngine* engine);
static void ibus_rime_engine_disable(IBusEngine* engine);
static void ibus_rime_engine_set_cursor_location(IBusEngine* engine, gint x, gint y, gint w, gint h);
static void ibus_rime_engine_set_capabilities(IBusEngine* engine, guint caps);
static void ibus_rime_engine_set_content_type(IBusEngine* engine, guint purpose, guint hint);
static void ibus_rime_engine_set_surrounding_text(IBusEngine* engine, IBusText* text, guint cursor_index, guint anchor_pos);
static void ibus_rime_engine_page_up(IBusEngine* engine);
static void ibus_rime_engine_page_down(IBusEngine* engine);
static void ibus_rime_engine_cursor_up(IBusEngine* engine);
static void ibus_rime_engine_cursor_down(IBusEngine* engine);
static void ibus_rime_engine_candidate_clicked(IBusEngine* engine, guint index, guint button, guint state);
static void ibus_rime_engine_property_activate(IBusEngine* engine, const gchar* prop_name, guint prop_state);
static void ibus_rime_engine_property_show(IBusEngine* engine, const gchar* prop_name);
static void ibus_rime_engine_property_hide(IBusEngine* engine, const gchar* prop_name);

// static void ibus_rime_engine_update(IBusRimeEngine* rime_engine);

G_DEFINE_TYPE(IBusRimeEngine, ibus_rime_engine, IBUS_TYPE_ENGINE)

static void ibus_rime_engine_class_init(IBusRimeEngineClass* klass) {
  IBusObjectClass* ibus_object_class = IBUS_OBJECT_CLASS(klass);
  ibus_object_class->destroy = (IBusObjectDestroyFunc)ibus_rime_engine_destroy;

  IBusEngineClass* engine_class = IBUS_ENGINE_CLASS(klass);
  engine_class->process_key_event = ibus_rime_engine_process_key_event;
  engine_class->focus_in = ibus_rime_engine_focus_in;
  engine_class->focus_out = ibus_rime_engine_focus_out;
  engine_class->reset = ibus_rime_engine_reset;
  engine_class->enable = ibus_rime_engine_enable;
  engine_class->disable = ibus_rime_engine_disable;
  engine_class->property_activate = ibus_rime_engine_property_activate;
  engine_class->candidate_clicked = ibus_rime_engine_candidate_clicked;
  engine_class->page_up = ibus_rime_engine_page_up;
  engine_class->page_down = ibus_rime_engine_page_down;
  engine_class->set_cursor_location = ibus_rime_engine_set_cursor_location;
  engine_class->set_capabilities = ibus_rime_engine_set_capabilities;
  engine_class->set_content_type = ibus_rime_engine_set_content_type;
  engine_class->set_surrounding_text = ibus_rime_engine_set_surrounding_text;
}

static void ibus_rime_engine_init(IBusRimeEngine* rime_engine) {
  log_printf("[debug] ibus_rime_engine_init\n");

  // rime_engine->table = ibus_lookup_table_new(9, 0, TRUE, FALSE);
  // g_object_ref_sink(rime_engine->table);

  // rime_engine->props = ibus_prop_list_new();
  // g_object_ref_sink(rime_engine->props);

  rime_engine->window = nullptr;
}

static void ibus_rime_engine_destroy(IBusRimeEngine* rime_engine) {
  log_printf("[debug] ibus_rime_engine_destroy\n");

  delete rime_engine->window;

  // if (rime_engine->table) {
  //   g_object_unref(rime_engine->table);
  //   rime_engine->table = NULL;
  // }

  // if (rime_engine->props) {
  //   g_object_unref(rime_engine->props);
  //   rime_engine->props = NULL;
  // }

  ((IBusObjectClass*)ibus_rime_engine_parent_class)->destroy((IBusObject*)rime_engine);
}

static void ibus_rime_engine_focus_in(IBusEngine* engine) {
  log_printf("[debug] ibus_rime_engine_focus_in\n");
}

static void ibus_rime_engine_focus_out(IBusEngine* engine) {
  log_printf("[debug] ibus_rime_engine_focus_out\n");
}

static void ibus_rime_engine_reset(IBusEngine* engine) {
  log_printf("[debug] ibus_rime_engine_reset\n");

  IBusRimeEngine* rime_engine = (IBusRimeEngine*)engine;

  rime_engine->text = "";
}

static void ibus_rime_engine_enable(IBusEngine* engine) {
  log_printf("[debug] ibus_rime_engine_enable\n");

  log_printf("[debug] engine.client_capabilities: %d\n", engine->client_capabilities);
  log_printf("[debug] engine.cursor_area: { x: %d, y: %d, w: %d, h: %d }\n", engine->cursor_area.x, engine->cursor_area.y, engine->cursor_area.width, engine->cursor_area.height);
  log_printf("[debug] engine.enabled: %d\n", engine->enabled);
  log_printf("[debug] engine.has_focus: %d\n", engine->has_focus);
}

static void ibus_rime_engine_disable(IBusEngine* engine) {
  log_printf("[debug] ibus_rime_engine_disable\n");
}

// static void ibus_rime_update_status(IBusRimeEngine* rime_engine, RimeStatus* status) {
//   if (status && rime_engine->status.is_disabled == status->is_disabled &&
//       rime_engine->status.is_ascii_mode == status->is_ascii_mode && rime_engine->status.schema_id &&
//       status->schema_id && !strcmp(rime_engine->status.schema_id, status->schema_id)) {
//     // no updates
//     return;
//   }

//   rime_engine->status.is_disabled = status ? status->is_disabled : False;
//   rime_engine->status.is_ascii_mode = status ? status->is_ascii_mode : False;
//   if (rime_engine->status.schema_id) {
//     g_free(rime_engine->status.schema_id);
//   }
//   rime_engine->status.schema_id = status && status->schema_id ? g_strdup(status->schema_id) : NULL;

//   IBusProperty* prop = ibus_prop_list_get(rime_engine->props, 0);
//   const gchar* icon;
//   IBusText* label;
//   if (prop) {
//     if (!status || status->is_disabled) {
//       icon = IBUS_RIME_ICONS_DIR "/disabled.png";
//       label = ibus_text_new_from_static_string("維護");
//     } else if (status->is_ascii_mode) {
//       icon = IBUS_RIME_ICONS_DIR "/abc.png";
//       label = ibus_text_new_from_static_string("Abc");
//     } else {
//       icon = IBUS_RIME_ICONS_DIR "/zh.png";
//       /* schema_name is ".default" in switcher */
//       if (status->schema_name && status->schema_name[0] != '.') {
//         label = ibus_text_new_from_string(status->schema_name);
//       } else {
//         label = ibus_text_new_from_static_string("中文");
//       }
//     }
//     if (status && !status->is_disabled && ibus_text_get_length(label) > 0) {
//       gunichar c = g_utf8_get_char(ibus_text_get_text(label));
//       IBusText* symbol = ibus_text_new_from_unichar(c);
//       ibus_property_set_symbol(prop, symbol);
//     }
//     ibus_property_set_icon(prop, icon);
//     ibus_property_set_label(prop, label);
//     ibus_engine_update_property((IBusEngine*)rime_engine, prop);
//   }
// }

// static void ibus_rime_engine_update(IBusRimeEngine* rime_engine) {
//   // update properties
//   RIME_STRUCT(RimeStatus, status);
//   if (rime_api->get_status(rime_engine->session_id, &status)) {
//     ibus_rime_update_status(rime_engine, &status);
//     rime_api->free_status(&status);
//   } else {
//     ibus_rime_update_status(rime_engine, NULL);
//   }

//   // commit text
//   RIME_STRUCT(RimeCommit, commit);
//   if (rime_api->get_commit(rime_engine->session_id, &commit)) {
//     IBusText* text;
//     text = ibus_text_new_from_string(commit.text);
//     // the text object will be released by ibus
//     ibus_engine_commit_text((IBusEngine*)rime_engine, text);
//     rime_api->free_commit(&commit);
//   }

//   // begin updating UI

//   RIME_STRUCT(RimeContext, context);
//   if (!rime_api->get_context(rime_engine->session_id, &context) || context.composition.length == 0) {
//     ibus_engine_hide_preedit_text((IBusEngine*)rime_engine);
//     ibus_engine_hide_auxiliary_text((IBusEngine*)rime_engine);
//     ibus_engine_hide_lookup_table((IBusEngine*)rime_engine);
//     rime_api->free_context(&context);
//     return;
//   }

//   IBusText* inline_text = NULL;
//   IBusText* auxiliary_text = NULL;
//   guint inline_cursor_pos = 0;
//   int preedit_offset = 0;

//   const gboolean has_highlighted_span = (context.composition.sel_start < context.composition.sel_end);

//   // display preview text inline, if the commit_text_preview API is supported.
//   if (g_ibus_rime_settings.embed_preedit_text && g_ibus_rime_settings.preedit_style == PREEDIT_STYLE_PREVIEW &&
//       RIME_STRUCT_HAS_MEMBER(context, context.commit_text_preview) && context.commit_text_preview) {
//     inline_text = ibus_text_new_from_string(context.commit_text_preview);
//     const guint inline_text_len = ibus_text_get_length(inline_text);
//     inline_cursor_pos = g_ibus_rime_settings.cursor_type == CURSOR_TYPE_SELECT
//         ? g_utf8_strlen(context.composition.preedit, context.composition.sel_start)
//         : inline_text_len;
//     inline_text->attrs = ibus_attr_list_new();
//     ibus_attr_list_append(inline_text->attrs, ibus_attr_underline_new(IBUS_ATTR_UNDERLINE_SINGLE, 0, inline_text_len));
//     // show the unconverted range of preedit text as auxiliary text
//     if (has_highlighted_span) {
//       preedit_offset = context.composition.sel_start;
//       if (g_ibus_rime_settings.color_scheme) {
//         const guint start = g_utf8_strlen(context.composition.preedit, context.composition.sel_start);
//         const guint end = inline_text_len;
//         ibus_attr_list_append(
//             inline_text->attrs, ibus_attr_foreground_new(g_ibus_rime_settings.color_scheme->text_color, start, end));
//         ibus_attr_list_append(
//             inline_text->attrs, ibus_attr_background_new(g_ibus_rime_settings.color_scheme->back_color, start, end));
//       }
//     } else {
//       // hide auxiliary text
//       preedit_offset = context.composition.length;
//     }
//   }
//   // display preedit text inline
//   else if (g_ibus_rime_settings.embed_preedit_text && g_ibus_rime_settings.preedit_style == PREEDIT_STYLE_COMPOSITION) {
//     inline_text = ibus_text_new_from_string(context.composition.preedit);
//     const guint inline_text_len = ibus_text_get_length(inline_text);
//     inline_cursor_pos = g_ibus_rime_settings.cursor_type == CURSOR_TYPE_SELECT
//         ? g_utf8_strlen(context.composition.preedit, context.composition.sel_start)
//         : g_utf8_strlen(context.composition.preedit, context.composition.cursor_pos);
//     inline_text->attrs = ibus_attr_list_new();
//     ibus_attr_list_append(inline_text->attrs, ibus_attr_underline_new(IBUS_ATTR_UNDERLINE_SINGLE, 0, inline_text_len));
//     if (has_highlighted_span && g_ibus_rime_settings.color_scheme) {
//       const guint start = g_utf8_strlen(context.composition.preedit, context.composition.sel_start);
//       const glong end = g_utf8_strlen(context.composition.preedit, context.composition.sel_end);
//       ibus_attr_list_append(
//           inline_text->attrs, ibus_attr_foreground_new(g_ibus_rime_settings.color_scheme->text_color, start, end));
//       ibus_attr_list_append(
//           inline_text->attrs, ibus_attr_background_new(g_ibus_rime_settings.color_scheme->back_color, start, end));
//     }
//     preedit_offset = context.composition.length;
//   }

//   // calculate auxiliary text
//   if (preedit_offset < context.composition.length) {
//     const char* preedit = context.composition.preedit + preedit_offset;
//     auxiliary_text = ibus_text_new_from_string(preedit);
//     // glong preedit_len = g_utf8_strlen(preedit, -1);
//     // glong cursor_pos = g_utf8_strlen(
//     //    preedit, context.composition.cursor_pos - preedit_offset);
//     if (has_highlighted_span) {
//       auxiliary_text->attrs = ibus_attr_list_new();
//       const glong start = g_utf8_strlen(preedit, context.composition.sel_start - preedit_offset);
//       const glong end = g_utf8_strlen(preedit, context.composition.sel_end - preedit_offset);
//       ibus_attr_list_append(auxiliary_text->attrs, ibus_attr_foreground_new(RIME_COLOR_BLACK, start, end));
//       ibus_attr_list_append(auxiliary_text->attrs, ibus_attr_background_new(RIME_COLOR_LIGHT, start, end));
//     }
//   }

//   if (inline_text) {
//     ibus_engine_update_preedit_text((IBusEngine*)rime_engine, inline_text, inline_cursor_pos, TRUE);
//   } else {
//     ibus_engine_hide_preedit_text((IBusEngine*)rime_engine);
//   }
//   if (auxiliary_text) {
//     ibus_engine_update_auxiliary_text((IBusEngine*)rime_engine, auxiliary_text, TRUE);
//   } else {
//     ibus_engine_hide_auxiliary_text((IBusEngine*)rime_engine);
//   }

//   ibus_lookup_table_clear(rime_engine->table);
//   if (context.menu.num_candidates) {
//     int i;
//     int num_select_keys = context.menu.select_keys ? strlen(context.menu.select_keys) : 0;
//     gboolean has_labels = RIME_STRUCT_HAS_MEMBER(context, context.select_labels) && context.select_labels;
//     gboolean has_page_down = !context.menu.is_last_page;
//     gboolean has_page_up = context.menu.is_last_page && context.menu.page_no > 0;
//     ibus_lookup_table_set_round(rime_engine->table, !(context.menu.is_last_page || context.menu.page_no == 0));
//     ibus_lookup_table_set_page_size(rime_engine->table, context.menu.page_size);
//     if (has_page_up) { // show page up for last page
//       for (i = 0; i < context.menu.page_size; ++i) {
//         ibus_lookup_table_append_candidate(rime_engine->table, ibus_text_new_from_string(""));
//       }
//     }
//     for (i = 0; i < context.menu.num_candidates; ++i) {
//       gchar* text = context.menu.candidates[i].text;
//       gchar* comment = context.menu.candidates[i].comment;
//       IBusText* cand_text = NULL;
//       if (comment) {
//         gchar* temp = g_strconcat(text, " ", comment, NULL);
//         cand_text = ibus_text_new_from_string(temp);
//         g_free(temp);
//         int text_len = g_utf8_strlen(text, -1);
//         int end_index = ibus_text_get_length(cand_text);
//         ibus_text_append_attribute(cand_text, IBUS_ATTR_TYPE_FOREGROUND, RIME_COLOR_DARK, text_len, end_index);
//       } else {
//         cand_text = ibus_text_new_from_string(text);
//       }
//       ibus_lookup_table_append_candidate(rime_engine->table, cand_text);
//       IBusText* label = NULL;
//       if (i < context.menu.page_size && has_labels) {
//         label = ibus_text_new_from_string(context.select_labels[i]);
//       } else if (i < num_select_keys) {
//         label = ibus_text_new_from_unichar(context.menu.select_keys[i]);
//       } else {
//         label = ibus_text_new_from_printf("%d", (i + 1) % 10);
//       }
//       ibus_lookup_table_set_label(rime_engine->table, i, label);
//     }
//     if (has_page_down) { // show page down except last page
//       ibus_lookup_table_append_candidate(rime_engine->table, ibus_text_new_from_string(""));
//     }
//     if (has_page_up) { // show page up for last page
//       ibus_lookup_table_set_cursor_pos(
//           rime_engine->table, context.menu.page_size + context.menu.highlighted_candidate_index);
//     } else {
//       ibus_lookup_table_set_cursor_pos(rime_engine->table, context.menu.highlighted_candidate_index);
//     }
//     ibus_lookup_table_set_orientation(rime_engine->table, g_ibus_rime_settings.lookup_table_orientation);
//     ibus_engine_update_lookup_table((IBusEngine*)rime_engine, rime_engine->table, TRUE);
//   } else {
//     ibus_engine_hide_lookup_table((IBusEngine*)rime_engine);
//   }

//   // end updating UI
//   rime_api->free_context(&context);
// }

static gboolean ibus_rime_engine_process_key_event(IBusEngine* engine, guint keyval, guint keycode, guint modifiers) {
  log_printf("[debug] ibus_rime_engine_process_key_event keyval:%c keycode:%d modifiers:%d\n", keyval, keycode, modifiers);

  IBusRimeEngine* rime_engine = (IBusRimeEngine*)engine;

  if (modifiers & IBUS_SUPER_MASK) {
    return FALSE;
  }

  if (modifiers & IBUS_RELEASE_MASK) {
    return FALSE;
  }

  // if (keyval == '\n') {
  //   return TRUE;
  // }

  if (keycode != 29 && keycode != 42) {
    if (keycode == 14) {
      if (rime_engine->text.length()) {
        rime_engine->text.erase(rime_engine->text.length() - 1);
      }
    } else {
      rime_engine->text += (char)keyval;
    }

    log_printf("[debug] text: %s\n", rime_engine->text.data());
  }

  modifiers &= (IBUS_RELEASE_MASK | IBUS_LOCK_MASK | IBUS_SHIFT_MASK | IBUS_CONTROL_MASK | IBUS_MOD1_MASK);

  return FALSE;
}

static void ibus_rime_engine_property_activate(IBusEngine* engine, const gchar* prop_name, guint prop_state) {
  log_printf("[debug] ibus_rime_engine_property_activate prop_name:%s prop_state:%d\n", prop_name, prop_state);
}

static void ibus_rime_engine_candidate_clicked(IBusEngine* engine, guint index, guint button, guint state) {
  log_printf("[debug] ibus_rime_engine_candidate_clicked index:%d button:%d state:%d\n", index, button, state);

  // IBusRimeEngine* rime_engine = (IBusRimeEngine*)engine;
  //
  // if (RIME_API_AVAILABLE(rime_api, select_candidate)) {
  //   RIME_STRUCT(RimeContext, context);
  //   if (!rime_api->get_context(rime_engine->session_id, &context) || context.composition.length == 0) {
  //     rime_api->free_context(&context);
  //     return;
  //   }
  //   rime_api->select_candidate(rime_engine->session_id, context.menu.page_no * context.menu.page_size + index);
  //   rime_api->free_context(&context);
  //   ibus_rime_engine_update(rime_engine);
  // }
}

static void ibus_rime_engine_page_up(IBusEngine* engine) {
  log_printf("[debug] ibus_rime_engine_page_up\n");

  // IBusRimeEngine* rime_engine = (IBusRimeEngine*)engine;
  //
  // rime_api->process_key(rime_engine->session_id, IBUS_KEY_Page_Up, 0);
  // ibus_rime_engine_update(rime_engine);
}

static void ibus_rime_engine_page_down(IBusEngine* engine) {
  log_printf("[debug] ibus_rime_engine_page_down\n");

  // IBusRimeEngine* rime_engine = (IBusRimeEngine*)engine;
  //
  // rime_api->process_key(rime_engine->session_id, IBUS_KEY_Page_Down, 0);
  // ibus_rime_engine_update(rime_engine);
}

static void ibus_rime_engine_set_cursor_location(IBusEngine* engine, gint x, gint y, gint w, gint h) {
  log_printf("[debug] ibus_rime_engine_set_cursor_location x:%d y:%d w:%d h:%d\n", x, y, w, h);

  IBusRimeEngine* rime_engine = (IBusRimeEngine*)engine;

  runInGUIThread([=]() {
    log_printf("TEST\n");

    if (!rime_engine->window) {
      rime_engine->window = new QMainWindow();
      rime_engine->window->resize(360, 200);
      rime_engine->window->setWindowOpacity(0.95);
      rime_engine->window->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
      // rime_engine->window->setWindowIcon(QIcon(":/res/x11-emoji-picker.png"));
    }

    // rime_engine->window->move(x, y);
    rime_engine->window->show();
  });
}

static void ibus_rime_engine_set_capabilities(IBusEngine* engine, guint caps) {
  log_printf("[debug] ibus_rime_engine_set_capabilities caps:%d\n", caps);
}

static void ibus_rime_engine_set_content_type(IBusEngine* engine, guint purpose, guint hint) {
  log_printf("[debug] ibus_rime_engine_set_content_type purpose:%d hint:%d\n", purpose, hint);
}

static void ibus_rime_engine_set_surrounding_text(IBusEngine* engine, IBusText* text, guint cursor_index, guint anchor_pos) {
  log_printf("[debug] ibus_rime_engine_set_surrounding_text text:%s cursor_index:%d anchor_pos:%d\n", text->text, cursor_index, anchor_pos);
}
}
