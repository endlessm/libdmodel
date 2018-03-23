/* Copyright 2016 Endless Mobile, Inc. */

#include "dm-image.h"

#include "dm-utils-private.h"
#include "dm-media-private.h"

#include <endless/endless.h>

/**
 * SECTION:image
 * @title: Image
 * @short_description: Access image object metadata
 *
 * The model class for image objects.
 */
G_DEFINE_TYPE (DmImage, dm_image, DM_TYPE_MEDIA)

static void
dm_image_class_init (G_GNUC_UNUSED DmImageClass *klass)
{
}

static void
dm_image_init (G_GNUC_UNUSED DmImage *self)
{
}

/**
 * dm_image_new_from_json_node:
 * @node: a json node with the model metadata
 *
 * Instantiates a #DmContent from a JsonNode of object metadata.
 * Outside of testing this metadata is usually retrieved from a shard.
 *
 * Returns: The newly created #DmImage.
 */
DmContent *
dm_image_new_from_json_node (JsonNode *node)
{
  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/object/image");

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GArray *params = g_array_new (FALSE, TRUE, sizeof (GParameter));
  dm_media_add_json_to_params (node, params);
  DmImage *model = g_object_newv (DM_TYPE_IMAGE, params->len,
                                  (GParameter *)params->data);
  dm_utils_free_gparam_array (params);
G_GNUC_END_IGNORE_DEPRECATIONS

  return DM_CONTENT (model);
}
