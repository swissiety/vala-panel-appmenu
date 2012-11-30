#include "unity-gtk-menu-shell-private.h"
#include "unity-gtk-menu-section-private.h"

G_DEFINE_TYPE (UnityGtkMenuShell, unity_gtk_menu_shell, G_TYPE_MENU_MODEL);

enum
{
  MENU_SHELL_PROP_0,
  MENU_SHELL_PROP_MENU_SHELL,
  MENU_SHELL_PROP_ITEMS,
  MENU_SHELL_PROP_SECTIONS,
  MENU_SHELL_PROP_VISIBLE_INDICES,
  MENU_SHELL_PROP_SEPARATOR_INDICES,
  MENU_SHELL_N_PROPERTIES
};

static GParamSpec *menu_shell_properties[MENU_SHELL_N_PROPERTIES] = { NULL };

static void
unity_gtk_menu_shell_handle_insert (GtkMenuShell *menu_shell,
                                    GtkWidget    *child,
                                    gint          position,
                                    gpointer      user_data)
{
}

static void
unity_gtk_menu_shell_set_menu_shell (UnityGtkMenuShell *shell,
                                     GtkMenuShell      *menu_shell)
{
  g_return_if_fail (UNITY_GTK_IS_MENU_SHELL (shell));

  if (menu_shell != shell->menu_shell)
    {
      GPtrArray *items = shell->items;
      GPtrArray *sections = shell->sections;
      GSequence *separator_indices = shell->separator_indices;

      if (shell->menu_shell_insert_handler_id)
        {
          g_assert (shell->menu_shell != NULL);
          g_signal_handler_disconnect (shell->menu_shell, shell->menu_shell_insert_handler_id);
          shell->menu_shell_insert_handler_id = 0;
        }

      if (separator_indices != NULL)
        {
          shell->separator_indices = NULL;
          g_sequence_free (separator_indices);
        }

      if (sections != NULL)
        {
          shell->sections = NULL;
          g_ptr_array_unref (sections);
        }

      if (items != NULL)
        {
          shell->items = NULL;
          g_ptr_array_unref (items);
        }

      shell->menu_shell = menu_shell;

      if (menu_shell != NULL)
        shell->menu_shell_insert_handler_id = g_signal_connect (menu_shell, "insert", G_CALLBACK (unity_gtk_menu_shell_handle_insert), shell);
    }
}

static GPtrArray *
unity_gtk_menu_shell_get_items (UnityGtkMenuShell *shell)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SHELL (shell), NULL);

  if (shell->items == NULL)
    {
      GList *iter;

      g_return_val_if_fail (shell->menu_shell != NULL, NULL);

      shell->items = g_ptr_array_new_with_free_func (g_object_unref);
      iter = gtk_container_get_children (GTK_CONTAINER (shell->menu_shell));

      while (iter != NULL)
        {
          g_ptr_array_add (shell->items, unity_gtk_menu_item_new (iter->data, shell));
          iter = g_list_next (iter);
        }
    }

  return shell->items;
}

static GPtrArray *
unity_gtk_menu_shell_get_sections (UnityGtkMenuShell *shell)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SHELL (shell), NULL);

  if (shell->sections == NULL)
    {
      GSequence *separator_indices = unity_gtk_menu_shell_get_separator_indices (shell);
      guint n = g_sequence_get_length (separator_indices);
      guint i;

      shell->sections = g_ptr_array_new_full (n + 1, g_object_unref);

      for (i = 0; i <= n; i++)
        g_ptr_array_add (shell->sections, unity_gtk_menu_section_new (shell, i));
    }

  return shell->sections;
}

static void
unity_gtk_menu_shell_dispose (GObject *object)
{
  UnityGtkMenuShell *shell;

  g_return_if_fail (UNITY_GTK_IS_MENU_SHELL (object));

  shell = UNITY_GTK_MENU_SHELL (object);

  unity_gtk_menu_shell_set_menu_shell (shell, NULL);

  G_OBJECT_CLASS (unity_gtk_menu_shell_parent_class)->dispose (object);
}

static void
unity_gtk_menu_shell_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  UnityGtkMenuShell *self;

  g_return_if_fail (UNITY_GTK_IS_MENU_SHELL (object));

  self = UNITY_GTK_MENU_SHELL (object);

  switch (property_id)
    {
    case MENU_SHELL_PROP_MENU_SHELL:
      g_value_set_pointer (value, self->menu_shell);
      break;

    case MENU_SHELL_PROP_ITEMS:
      g_value_set_pointer (value, unity_gtk_menu_shell_get_items (self));
      break;

    case MENU_SHELL_PROP_SECTIONS:
      g_value_set_pointer (value, unity_gtk_menu_shell_get_sections (self));
      break;

    case MENU_SHELL_PROP_VISIBLE_INDICES:
      g_value_set_pointer (value, unity_gtk_menu_shell_get_visible_indices (self));
      break;

    case MENU_SHELL_PROP_SEPARATOR_INDICES:
      g_value_set_pointer (value, unity_gtk_menu_shell_get_separator_indices (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
unity_gtk_menu_shell_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  UnityGtkMenuShell *self;

  g_return_if_fail (UNITY_GTK_IS_MENU_SHELL (object));

  self = UNITY_GTK_MENU_SHELL (object);

  switch (property_id)
    {
    case MENU_SHELL_PROP_MENU_SHELL:
      unity_gtk_menu_shell_set_menu_shell (self, g_value_get_pointer (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
unity_gtk_menu_shell_is_mutable (GMenuModel *model)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SHELL (model), TRUE);

  return TRUE;
}

static gint
unity_gtk_menu_shell_get_n_items (GMenuModel *model)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SHELL (model), 0);

  return unity_gtk_menu_shell_get_sections (UNITY_GTK_MENU_SHELL (model))->len;
}

static void
unity_gtk_menu_shell_get_item_attributes (GMenuModel  *model,
                                          gint         item_index,
                                          GHashTable **attributes)
{
  g_return_if_fail (UNITY_GTK_IS_MENU_SHELL (model));
  g_return_if_fail (0 <= item_index && item_index < g_menu_model_get_n_items (model));
  g_return_if_fail (attributes != NULL);

  *attributes = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify) g_variant_unref);
}

static void
unity_gtk_menu_shell_get_item_links (GMenuModel  *model,
                                     gint         item_index,
                                     GHashTable **links)
{
  UnityGtkMenuShell *shell;
  GPtrArray *sections;
  UnityGtkMenuSection *section;

  g_return_if_fail (UNITY_GTK_IS_MENU_SHELL (model));
  g_return_if_fail (0 <= item_index && item_index < g_menu_model_get_n_items (model));
  g_return_if_fail (links != NULL);

  shell = UNITY_GTK_MENU_SHELL (model);
  sections = unity_gtk_menu_shell_get_sections (shell);
  section = g_ptr_array_index (sections, item_index);
  *links = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);
  g_hash_table_insert (*links, G_MENU_LINK_SECTION, g_object_ref (section));
}

static void
unity_gtk_menu_shell_class_init (UnityGtkMenuShellClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GMenuModelClass *menu_model_class = G_MENU_MODEL_CLASS (klass);

  object_class->dispose = unity_gtk_menu_shell_dispose;
  object_class->get_property = unity_gtk_menu_shell_get_property;
  object_class->set_property = unity_gtk_menu_shell_set_property;
  menu_model_class->is_mutable = unity_gtk_menu_shell_is_mutable;
  menu_model_class->get_n_items = unity_gtk_menu_shell_get_n_items;
  menu_model_class->get_item_attributes = unity_gtk_menu_shell_get_item_attributes;
  menu_model_class->get_item_links = unity_gtk_menu_shell_get_item_links;

  menu_shell_properties[MENU_SHELL_PROP_MENU_SHELL] = g_param_spec_pointer ("menu-shell",
                                                                            "Menu shell",
                                                                            "Menu shell",
                                                                            G_PARAM_READWRITE);
  menu_shell_properties[MENU_SHELL_PROP_ITEMS] = g_param_spec_pointer ("items",
                                                                       "Items",
                                                                       "Items",
                                                                       G_PARAM_READABLE);
  menu_shell_properties[MENU_SHELL_PROP_SECTIONS] = g_param_spec_pointer ("sections",
                                                                          "Sections",
                                                                          "Sections",
                                                                          G_PARAM_READABLE);
  menu_shell_properties[MENU_SHELL_PROP_VISIBLE_INDICES] = g_param_spec_pointer ("visible-indices",
                                                                                 "Visible indices",
                                                                                 "Visible indices",
                                                                                 G_PARAM_READABLE);
  menu_shell_properties[MENU_SHELL_PROP_SEPARATOR_INDICES] = g_param_spec_pointer ("separator-indices",
                                                                                   "Separator indices",
                                                                                   "Separator indices",
                                                                                   G_PARAM_READABLE);

  g_object_class_install_properties (object_class, MENU_SHELL_N_PROPERTIES, menu_shell_properties);
}

static void
unity_gtk_menu_shell_init (UnityGtkMenuShell *self)
{
}

UnityGtkMenuShell *
unity_gtk_menu_shell_new (GtkMenuShell *menu_shell)
{
  return g_object_new (UNITY_GTK_TYPE_MENU_SHELL,
                       "menu-shell", menu_shell,
                       NULL);
}

UnityGtkMenuItem *
unity_gtk_menu_shell_get_item (UnityGtkMenuShell *shell,
                               guint              index)
{
  GPtrArray *items;

  g_return_val_if_fail (UNITY_GTK_IS_MENU_SHELL (shell), NULL);

  items = unity_gtk_menu_shell_get_items (shell);

  g_return_val_if_fail (0 <= index && index < items->len, NULL);

  return g_ptr_array_index (items, index);
}

GSequence *
unity_gtk_menu_shell_get_visible_indices (UnityGtkMenuShell *shell)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SHELL (shell), NULL);

  if (shell->visible_indices == NULL)
    {
      GPtrArray *items = unity_gtk_menu_shell_get_items (shell);
      guint i;

      shell->visible_indices = g_sequence_new (NULL);

      for (i = 0; i < items->len; i++)
        {
          UnityGtkMenuItem *item = g_ptr_array_index (items, i);

          if (unity_gtk_menu_item_is_visible (item))
            g_sequence_append (shell->visible_indices, GUINT_TO_POINTER (i));
        }
    }

  return shell->visible_indices;
}

GSequence *
unity_gtk_menu_shell_get_separator_indices (UnityGtkMenuShell *shell)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SHELL (shell), NULL);

  if (shell->separator_indices == NULL)
    {
      GPtrArray *items = unity_gtk_menu_shell_get_items (shell);
      guint i;

      shell->separator_indices = g_sequence_new (NULL);

      for (i = 0; i < items->len; i++)
        {
          UnityGtkMenuItem *item = g_ptr_array_index (items, i);

          if (unity_gtk_menu_item_is_visible (item) && unity_gtk_menu_item_is_separator (item))
            g_sequence_append (shell->separator_indices, GUINT_TO_POINTER (i));
        }
    }

  return shell->separator_indices;
}

gint
g_uintcmp (gconstpointer a,
           gconstpointer b,
           gpointer      user_data)
{
  return GPOINTER_TO_INT (a) - GPOINTER_TO_INT (b);
}