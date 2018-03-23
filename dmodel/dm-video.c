/* Copyright 2016 Endless Mobile, Inc. */

#include "dm-video.h"

#include "dm-utils-private.h"
#include "dm-media-private.h"

#include <endless/endless.h>

/**
 * SECTION:video
 * @title: Video
 * @short_description: Access video object metadata
 *
 * The model class for video objects.
 */
typedef struct {
  guint duration;
  gchar *transcript;
  gchar *poster_uri;
} DmVideoPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DmVideo, dm_video, DM_TYPE_MEDIA)

enum {
  PROP_0,
  PROP_DURATION,
  PROP_TRANSCRIPT,
  PROP_POSTER_URI,
  NPROPS
};

static GParamSpec *dm_video_props[NPROPS] = { NULL, };

static void
dm_video_get_property (GObject *object,
                       guint prop_id,
                       GValue *value,
                       GParamSpec *pspec)
{
  DmVideo *self = DM_VIDEO (object);
  DmVideoPrivate *priv = dm_video_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_DURATION:
      g_value_set_uint (value, priv->duration);
      break;

    case PROP_TRANSCRIPT:
      g_value_set_string (value, priv->transcript);
      break;

    case PROP_POSTER_URI:
      g_value_set_string (value, priv->poster_uri);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_video_set_property (GObject *object,
                       guint prop_id,
                       const GValue *value,
                       GParamSpec *pspec)
{
  DmVideo *self = DM_VIDEO (object);
  DmVideoPrivate *priv = dm_video_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_DURATION:
      priv->duration = g_value_get_uint (value);
      break;

    case PROP_TRANSCRIPT:
      g_clear_pointer (&priv->transcript, g_free);
      priv->transcript = g_value_dup_string (value);
      break;

    case PROP_POSTER_URI:
      g_clear_pointer (&priv->poster_uri, g_free);
      priv->poster_uri = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_video_finalize (GObject *object)
{
  DmVideo *self = DM_VIDEO (object);
  DmVideoPrivate *priv = dm_video_get_instance_private (self);

  g_clear_pointer (&priv->transcript, g_free);
  g_clear_pointer (&priv->poster_uri, g_free);

  G_OBJECT_CLASS (dm_video_parent_class)->finalize (object);
}

static void
dm_video_class_init (DmVideoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dm_video_get_property;
  object_class->set_property = dm_video_set_property;
  object_class->finalize = dm_video_finalize;

  /**
   * DmVideo:duration:
   *
   * The duration of the video in seconds.
   */
  dm_video_props[PROP_DURATION] =
    g_param_spec_uint ("duration", "Duration",
      "The duration of the video",
      0, G_MAXUINT, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  /**
   * DmVideo:transcript:
   *
   * Transcript of the video, in the same language as the video.
   */
  dm_video_props[PROP_TRANSCRIPT] =
    g_param_spec_string ("transcript", "Transcript",
      "Transcript of the video",
      "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  /**
   * DmVideo:poster-uri:
   *
   * URI of the video's poster image
   *
   * The EKN ID of an #Image.
   */
  dm_video_props[PROP_POSTER_URI] =
    g_param_spec_string ("poster-uri", "Poster URI",
      "URI of the poster image",
      "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, NPROPS, dm_video_props);
}

static void
dm_video_init (G_GNUC_UNUSED DmVideo *self)
{
}

static void
dm_video_add_json_to_params (JsonNode *node,
                             GArray *params)
{
  if (!JSON_NODE_HOLDS_OBJECT (node))
    {
      g_critical ("Trying to instantiate a DmVideo from a non json object.");
      return;
    }

  dm_media_add_json_to_params (node, params);

  JsonObject *object = json_node_get_object (node);
  GObjectClass *klass = g_type_class_ref (DM_TYPE_VIDEO);

  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "duration"),
                                         g_object_class_find_property (klass, "duration"),
                                         params);
  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "transcript"),
                                         g_object_class_find_property (klass, "transcript"),
                                         params);
  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "poster"),
                                         g_object_class_find_property (klass, "poster-uri"),
                                         params);
  g_type_class_unref (klass);
}

/**
 * dm_video_new_from_json_node:
 * @node: a json node with the model metadata
 *
 * Instantiates a #DmVideo from a JsonNode of object metadata.
 * Outside of testing this metadata is usually retrieved from a shard.
 *
 * Returns: The newly created #DmVideo.
 */
DmContent *
dm_video_new_from_json_node (JsonNode *node)
{
  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/object/video");

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GArray *params = g_array_new (FALSE, TRUE, sizeof (GParameter));
  dm_video_add_json_to_params (node, params);
  DmVideo *model = g_object_newv (DM_TYPE_VIDEO, params->len,
                                  (GParameter *)params->data);
  dm_utils_free_gparam_array (params);
G_GNUC_END_IGNORE_DEPRECATIONS

  return DM_CONTENT (model);
}
