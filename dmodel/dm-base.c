#include "dm-base.h"

#include "dm-audio.h"
#include "dm-article.h"
#include "dm-dictionary-entry.h"
#include "dm-set.h"
#include "dm-media.h"
#include "dm-image.h"
#include "dm-utils.h"
#include "dm-video.h"

#include <json-glib/json-glib.h>
#include <endless/endless.h>

/**
 * dm_model_from_json_node:
 * @node: a JsonNode with object metadata
 * @error: an error if one occurred
 *
 * Creates an object model.
 *
 * Returns: (transfer full): The newly created #DmContent.
 */
DmContent *
dm_model_from_json_node (JsonNode *node,
                         GError **error)
{
  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/base/from_json_node");

  if (!JSON_NODE_HOLDS_OBJECT (node))
    {
      g_set_error (error, DM_CONTENT_ERROR, DM_CONTENT_ERROR_BAD_FORMAT,
                   "Trying to instantiate a object model from a non json object");
      return NULL;
    }

  JsonNode *type_node = json_object_get_member (json_node_get_object (node), "@type");

  if (type_node == NULL)
    {
      g_set_error (error, DM_CONTENT_ERROR, DM_CONTENT_ERROR_BAD_FORMAT,
                   "Object model json has no @type field");
      return NULL;
    }

  if (!JSON_NODE_HOLDS_VALUE (type_node) ||
      json_node_get_value_type (type_node) != G_TYPE_STRING)
    {
      g_set_error (error, DM_CONTENT_ERROR, DM_CONTENT_ERROR_BAD_FORMAT,
                   "Unexpected value type for @type field");
      return NULL;
    }


  const gchar *type = json_node_get_string (type_node);
  if (g_strcmp0 (type, "ekn://_vocab/ContentObject") == 0)
    {
      return dm_content_new_from_json_node (node);
    }
  else if (g_strcmp0 (type, "ekn://_vocab/ArticleObject") == 0)
    {
      return dm_article_new_from_json_node (node);
    }
  else if (g_strcmp0 (type, "ekn://_vocab/DictionaryObject") == 0)
    {
      return dm_dictionary_entry_new_from_json_node (node);
    }
  else if (g_strcmp0 (type, "ekn://_vocab/SetObject") == 0)
    {
      return dm_set_new_from_json_node (node);
    }
  else if (g_strcmp0 (type, "ekn://_vocab/MediaObject") == 0)
    {
      return dm_media_new_from_json_node (node);
    }
  else if (g_strcmp0 (type, "ekn://_vocab/ImageObject") == 0)
    {
      return dm_image_new_from_json_node (node);
    }
  else if (g_strcmp0 (type, "ekn://_vocab/VideoObject") == 0)
    {
      return dm_video_new_from_json_node (node);
    }
  else if (g_strcmp0 (type, "ekn://_vocab/AudioObject") == 0)
    {
      return dm_audio_new_from_json_node (node);
    }
  else
    {
      g_set_error (error, DM_CONTENT_ERROR, DM_CONTENT_ERROR_BAD_FORMAT,
                   "Unknown value for @type field %s", type);
      return NULL;
    }
}
