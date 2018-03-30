/* Copyright 2017 Endless Mobile, Inc. */

#include "dm-audio.h"

#include "dm-utils-private.h"
#include "dm-content-private.h"

#include <endless/endless.h>

/**
 * SECTION:audio
 * @title: Audio
 * @short_description: Access audio object metadata
 *
 * The model class for audio objects.
 */
typedef struct {
  guint duration;
  gchar *transcript;
} DmAudioPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DmAudio, dm_audio, DM_TYPE_CONTENT)

enum {
  PROP_0,
  PROP_DURATION,
  PROP_TRANSCRIPT,
  NPROPS
};

static GParamSpec *dm_audio_props[NPROPS] = { NULL, };

static void
dm_audio_get_property (GObject *object,
                       guint prop_id,
                       GValue *value,
                       GParamSpec *pspec)
{
  DmAudio *self = DM_AUDIO (object);
  DmAudioPrivate *priv = dm_audio_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_DURATION:
      g_value_set_uint (value, priv->duration);
      break;

    case PROP_TRANSCRIPT:
      g_value_set_string (value, priv->transcript);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_audio_set_property (GObject *object,
                       guint prop_id,
                       const GValue *value,
                       GParamSpec *pspec)
{
  DmAudio *self = DM_AUDIO (object);
  DmAudioPrivate *priv = dm_audio_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_DURATION:
      priv->duration = g_value_get_uint (value);
      break;

    case PROP_TRANSCRIPT:
      g_clear_pointer (&priv->transcript, g_free);
      priv->transcript = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_audio_finalize (GObject *object)
{
  DmAudio *self = DM_AUDIO (object);
  DmAudioPrivate *priv = dm_audio_get_instance_private (self);

  g_clear_pointer (&priv->transcript, g_free);

  G_OBJECT_CLASS (dm_audio_parent_class)->finalize (object);
}

static void
dm_audio_class_init (DmAudioClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dm_audio_get_property;
  object_class->set_property = dm_audio_set_property;
  object_class->finalize = dm_audio_finalize;

  /**
   * DmAudio:duration:
   *
   * The duration of the audio in seconds.
   */
  dm_audio_props[PROP_DURATION] =
    g_param_spec_uint ("duration", "Duration",
      "The duration of the audio",
      0, G_MAXUINT, 0, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  /**
   * DmAudio:transcript:
   *
   * Transcript of the audio, in the same language as the audio.
   */
  dm_audio_props[PROP_TRANSCRIPT] =
    g_param_spec_string ("transcript", "Transcript",
      "Transcript of the audio",
      "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, NPROPS, dm_audio_props);
}

static void
dm_audio_init (G_GNUC_UNUSED DmAudio *self)
{
}

/**
 * dm_audio_add_json_to_params: (skip)
 * @node: a json node
 * @params: a caller owned array a gparams
 *
 * Private function. Appends GParameters to the array with DmArticle
 * property values parsed from the json node metadata.
 */
void
dm_audio_add_json_to_params (JsonNode *node,
                             GArray *params)
{
  if (!JSON_NODE_HOLDS_OBJECT (node))
    {
      g_critical ("Trying to instantiate a DmAudio from a non json object.");
      return;
    }

  dm_content_add_json_to_params (node, params);

  JsonObject *object = json_node_get_object (node);
  GObjectClass *klass = g_type_class_ref (DM_TYPE_AUDIO);

  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "duration"),
                                         g_object_class_find_property (klass, "duration"),
                                         params);
  dm_utils_append_gparam_from_json_node (json_object_get_member (object, "transcript"),
                                         g_object_class_find_property (klass, "transcript"),
                                         params);
  g_type_class_unref (klass);
}

/**
 * dm_audio_new_from_json_node:
 * @node: a json node with the model metadata
 *
 * Instantiates a #DmAudio from a JsonNode of object metadata.
 * Outside of testing this metadata is usually retrieved from a shard.
 *
 * Returns: The newly created #DmAudio.
 */
DmContent *
dm_audio_new_from_json_node (JsonNode *node)
{
  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/object/audio");

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GArray *params = g_array_new (FALSE, TRUE, sizeof (GParameter));
  dm_audio_add_json_to_params (node, params);
  DmAudio *model = g_object_newv (DM_TYPE_AUDIO, params->len,
                                  (GParameter *)params->data);
  dm_utils_free_gparam_array (params);
G_GNUC_END_IGNORE_DEPRECATIONS

  return DM_CONTENT (model);
}
