/* Copyright 2016 Endless Mobile, Inc. */

#include "dm-media.h"
#include "dm-media-private.h"

#include "dm-utils-private.h"
#include "dm-content-private.h"

#include <endless/endless.h>

/**
 * SECTION:media
 * @title: Media
 * @short_description: Access media object metadata
 *
 * The model class for media objects.
 * A media object has the same properties as a #DmContent model, plus caption,
 * height and width properties.
 */
typedef struct {
  gchar *caption;
  guint width;
  guint height;
  char *parent_uri;
} DmMediaPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DmMedia, dm_media, DM_TYPE_CONTENT)

enum {
  PROP_0,
  PROP_CAPTION,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_PARENT_URI,
  NPROPS
};

static GParamSpec *dm_media_props[NPROPS] = { NULL, };

static void
dm_media_get_property (GObject *object,
                       guint prop_id,
                       GValue *value,
                       GParamSpec *pspec)
{
  DmMedia *self = DM_MEDIA (object);
  DmMediaPrivate *priv = dm_media_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_CAPTION:
      g_value_set_string (value, priv->caption);
      break;

    case PROP_WIDTH:
      g_value_set_uint (value, priv->width);
      break;

    case PROP_HEIGHT:
      g_value_set_uint (value, priv->height);
      break;

    case PROP_PARENT_URI:
      g_value_set_string (value, priv->parent_uri);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_media_set_property (GObject *object,
                       guint prop_id,
                       const GValue *value,
                       GParamSpec *pspec)
{
  DmMedia *self = DM_MEDIA (object);
  DmMediaPrivate *priv = dm_media_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_CAPTION:
      g_clear_pointer (&priv->caption, g_free);
      priv->caption = g_value_dup_string (value);
      break;

    case PROP_WIDTH:
      priv->width = g_value_get_uint (value);
      break;

    case PROP_HEIGHT:
      priv->height = g_value_get_uint (value);
      break;

    case PROP_PARENT_URI:
      g_clear_pointer (&priv->parent_uri, g_free);
      priv->parent_uri = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_media_finalize (GObject *object)
{
  DmMedia *self = DM_MEDIA (object);
  DmMediaPrivate *priv = dm_media_get_instance_private (self);

  g_clear_pointer (&priv->caption, g_free);
  g_clear_pointer (&priv->parent_uri, g_free);

  G_OBJECT_CLASS (dm_media_parent_class)->finalize (object);
}

static void
dm_media_class_init (DmMediaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dm_media_get_property;
  object_class->set_property = dm_media_set_property;
  object_class->finalize = dm_media_finalize;

  /**
   * DmMedia:caption:
   *
   * A displayable string which describes the media object in the same
   * language as the MediaObject.
   */
  dm_media_props[PROP_CAPTION] =
    g_param_spec_string ("caption", "Caption",
      "Displayable caption for the media",
      "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  /**
   * DmMedia:width:
   *
   * The width of the media in pixels.
   */
  dm_media_props[PROP_WIDTH] =
    g_param_spec_uint ("width", "Width",
      "The width of the media in pixels",
      0, G_MAXUINT, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  /**
   * DmMedia:height:
   *
   * The height of the media in pixels.
   */
  dm_media_props[PROP_HEIGHT] =
    g_param_spec_uint ("height", "Height",
      "The height of the media in pixels",
      0, G_MAXUINT, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * DmMedia:parent-uri:
   *
   * The EKN URI of an #DmContent that embeds this
   * #DmMedia.
   */
  dm_media_props[PROP_PARENT_URI] =
    g_param_spec_string ("parent-uri", "Parent URI",
      "EKN URI of article that embeds this media object",
      NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     NPROPS,
                                     dm_media_props);
}

static void
dm_media_init (G_GNUC_UNUSED DmMedia *self)
{
}

/**
 * dm_media_add_json_to_params: (skip)
 * @node: a json node
 * @params: a caller owned array a gparams
 *
 * Private function. Appends GParameters to the array with DmArticle
 * property values parsed from the json node metadata.
 */
void
dm_media_add_json_to_params (JsonNode *node,
                             GArray *params)
{
  if (!JSON_NODE_HOLDS_OBJECT (node))
    {
      g_critical ("Trying to instantiate a DmMedia from a non json object.");
      return;
    }

  dm_content_add_json_to_params (node, params);

  JsonObject *object = json_node_get_object (node);
  GObjectClass *klass = g_type_class_ref (DM_TYPE_MEDIA);

  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "caption"),
                                         g_object_class_find_property (klass, "caption"),
                                         params);
  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "width"),
                                         g_object_class_find_property (klass, "width"),
                                         params);
  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "height"),
                                         g_object_class_find_property (klass, "height"),
                                         params);
  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "parent"),
                                         g_object_class_find_property (klass, "parent-uri"),
                                         params);
  g_type_class_unref (klass);
}

/**
 * dm_media_new_from_json_node:
 * @node: a json node with the model metadata
 *
 * Instantiates a #DmMedia from a JsonNode of object metadata.
 * Outside of testing this metadata is usually retrieved from a shard.
 *
 * Returns: The newly created #DmMedia.
 */
DmContent *
dm_media_new_from_json_node (JsonNode *node)
{
  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/object/media");

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GArray *params = g_array_new (FALSE, TRUE, sizeof (GParameter));
  dm_media_add_json_to_params (node, params);
  DmMedia *model = g_object_newv (DM_TYPE_MEDIA, params->len,
                                  (GParameter *)params->data);
  dm_utils_free_gparam_array (params);
G_GNUC_END_IGNORE_DEPRECATIONS

  return DM_CONTENT (model);
}
