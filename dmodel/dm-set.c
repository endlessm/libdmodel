/* Copyright 2016 Endless Mobile, Inc. */

#include "dm-set.h"

#include "dm-utils-private.h"
#include "dm-content-private.h"

#include <endless/endless.h>

/**
 * SECTION:set
 * @title: Set
 * @short_description: Access set object metadata
 *
 * The model class for set objects.
 */
typedef struct {
  char **child_tags;
} DmSetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DmSet, dm_set, DM_TYPE_CONTENT)

enum {
  PROP_0,
  PROP_CHILD_TAGS,
  NPROPS
};

static GParamSpec *dm_set_props[NPROPS] = { NULL, };

static void
dm_set_get_property (GObject *object,
                     guint prop_id,
                     GValue *value,
                     GParamSpec *pspec)
{
  DmSet *self = DM_SET (object);
  DmSetPrivate *priv = dm_set_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_CHILD_TAGS:
      g_value_set_boxed (value, priv->child_tags);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_set_set_property (GObject *object,
                     guint prop_id,
                     const GValue *value,
                     GParamSpec *pspec)
{
  DmSet *self = DM_SET (object);
  DmSetPrivate *priv = dm_set_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_CHILD_TAGS:
      g_clear_pointer (&priv->child_tags, g_strfreev);
      priv->child_tags = g_value_dup_boxed (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_set_finalize (GObject *object)
{
  DmSet *self = DM_SET (object);
  DmSetPrivate *priv = dm_set_get_instance_private (self);

  g_clear_pointer (&priv->child_tags, g_strfreev);

  G_OBJECT_CLASS (dm_set_parent_class)->finalize (object);
}

static void
dm_set_class_init (DmSetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dm_set_get_property;
  object_class->set_property = dm_set_set_property;
  object_class->finalize = dm_set_finalize;

  /**
   * DmSet:child-tags:
   *
   * A list of tags that articles in this set are tagged with
   *
   * child-tags refers to the articles that are contained within a
   * set; those articles are all articles whose
   * #Content:tags properties contain one of the tags in
   * the set's child-tags property.
   *
   * Note that #DmContent:tags on an #DmSet refers
   * to the tags with which a set itself has been tagged.
   */
  dm_set_props[PROP_CHILD_TAGS] =
    g_param_spec_boxed ("child-tags", "Child Tags",
      "A list of tags that articles in this set are tagged with",
      G_TYPE_STRV,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, NPROPS, dm_set_props);
}

static void
dm_set_init (G_GNUC_UNUSED DmSet *self)
{
}

static void
dm_set_add_json_to_params (JsonNode *node,
                           GArray *params)
{
  if (!JSON_NODE_HOLDS_OBJECT (node))
    {
      g_critical ("Trying to instantiate a DmSet from a non json object.");
      return;
    }

  dm_content_add_json_to_params (node, params);

  JsonObject *object = json_node_get_object (node);
  GObjectClass *klass = g_type_class_ref (DM_TYPE_SET);

  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "childTags"),
                                         g_object_class_find_property (klass, "child-tags"),
                                         params);
  g_type_class_unref (klass);
}

/**
 * dm_set_get_child_tags:
 * @self: the model
 *
 * Get the models child_tags.
 *
 * Returns: (transfer none) (array zero-terminated=1): an array of strings
 */
char * const *
dm_set_get_child_tags (DmSet *self)
{
  g_return_val_if_fail (DM_IS_SET (self), NULL);

  DmSetPrivate *priv = dm_set_get_instance_private (self);
  return priv->child_tags;
}

/**
 * dm_set_new_from_json_node:
 * @node: a json node with the model metadata
 *
 * Instantiates a #DmSet from a JsonNode of object metadata.
 * Outside of testing this metadata is usually retrieved from a shard.
 *
 * Returns: The newly created #DmSet.
 */
DmContent *
dm_set_new_from_json_node (JsonNode *node)
{
  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/object/set");

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GArray *params = g_array_new (FALSE, TRUE, sizeof (GParameter));
  dm_set_add_json_to_params (node, params);
  DmSet *model = g_object_newv (DM_TYPE_SET, params->len,
                                (GParameter *)params->data);
  dm_utils_free_gparam_array (params);
G_GNUC_END_IGNORE_DEPRECATIONS

  return DM_CONTENT (model);
}
