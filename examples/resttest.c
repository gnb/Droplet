/*
 * simple example which operates into a folder (creation of a folder, atomic file creation + get/set data/metadata + deletion + listing)
 */

#include <droplet.h>
#include <assert.h>

int
main(int argc,
     char **argv)
{
  int ret;
  dpl_ctx_t *ctx;
  char *folder = NULL;
  int folder_len;
  dpl_dict_t *metadata = NULL;
  char *data_buf = NULL;
  size_t data_len;
  char *data_buf_returned = NULL;
  u_int data_len_returned;
  dpl_dict_t *metadata_returned = NULL;
  dpl_dict_t *metadata2_returned = NULL;
  dpl_dict_var_t *metadatum = NULL;
  dpl_option_t option;
  char *id_resource = NULL;

  if (2 != argc)
    {
      fprintf(stderr, "usage: restrest folder\n");
      ret = 1;
      goto end;
    }

  folder = argv[1];
  folder_len = strlen(folder);
  if (folder_len < 1)
    {
      fprintf(stderr, "bad folder\n");
      ret = 1;
      goto end;
    }
  if (folder[folder_len-1] != '/')
    {
      fprintf(stderr, "folder name must end with a slash\n");
      ret = 1;
      goto end;
    }

  ret = dpl_init();           //init droplet library
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "dpl_init failed\n");
      ret = 1;
      goto end;
    }

  //open default profile
  ctx = dpl_ctx_new(NULL,     //droplet directory, default: "~/.droplet"
                    NULL);    //droplet profile, default:   "default"
  if (NULL == ctx)
    {
      fprintf(stderr, "dpl_ctx_new failed\n");
      ret = 1;
      goto free_dpl;
    }

  //ctx->trace_level = ~0;
  //ctx->trace_buffers = 1;

  /**/

  fprintf(stderr, "creating folder\n");

  ret = dpl_put(ctx,           //the context
                NULL,          //no bucket
                folder,        //the folder
                NULL,          //no subresource
                NULL,          //no option
                DPL_FTYPE_DIR, //directory
                NULL,          //no condition
                NULL,          //no metadata
                NULL,          //no sysmd
                NULL,          //object body
                0);            //object length
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "dpl_put failed: %s (%d)\n", dpl_status_str(ret), ret);
      ret = 1;
      goto free_all;
    }

  /**/

  data_len = 10000;
  data_buf = malloc(data_len);
  if (NULL == data_buf)
    {
      fprintf(stderr, "alloc data failed\n");
      ret = 1;
      goto free_all;
    }

  memset(data_buf, 'z', data_len);

  metadata = dpl_dict_new(13);
  if (NULL == metadata)
    {
      fprintf(stderr, "dpl_dict_new failed\n");
      ret = 1;
      goto free_all;
    }
 
  ret = dpl_dict_add(metadata, "foo", "bar", 0);
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "dpl_dict_add failed\n");
      ret = 1;
      goto free_all;
    }

  ret = dpl_dict_add(metadata, "foo2", "qux", 0);
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "dpl_dict_add failed\n");
      ret = 1;
      goto free_all;
    }

  /**/
  
  fprintf(stderr, "atomic creation of an object+MD\n");

  ret = dpl_post(ctx,           //the context
                 NULL,          //no bucket
                 folder,        //the folder
                 NULL,          //no subresource
                 NULL,          //no option
                 DPL_FTYPE_REG, //regular object
                 metadata,      //the metadata
                 NULL,          //no sysmd
                 data_buf,      //object body
                 data_len,      //object length
                 NULL,          //no query params
                 &id_resource); //the id resource path
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "dpl_post failed: %s (%d)\n", dpl_status_str(ret), ret);
      ret = 1;
      goto free_all;
    }

  /**/

  fprintf(stderr, "id resource path (unique): %s\n", id_resource);

  /**/

#if 0
  fprintf(stderr, "getting object+MD\n");

  option.mask = DPL_OPTION_LAZY; //enable this for faster GETs

  ret = dpl_get_id(ctx,           //the context
                   NULL,          //no bucket
                   id,            //the key
                   0,
                   NULL,          //no subresource
                   &option,
                   DPL_FTYPE_REG, //object type
                   NULL,
                   &data_buf_returned,  //data object
                   &data_len_returned,  //data object length
                   &metadata_returned, //metadata
                   NULL);              //sysmd
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "dpl_get_id failed: %s (%d)\n", dpl_status_str(ret), ret);
      ret = 1;
      goto free_all;
    }

  fprintf(stderr, "checking object\n");

  if (data_len != data_len_returned)
    {
      fprintf(stderr, "data lengths mismatch\n");
      ret = 1;
      goto free_all;
    }

  if (0 != memcmp(data_buf, data_buf_returned, data_len))
    {
      fprintf(stderr, "data content mismatch\n");
      ret = 1;
      goto free_all;
    }

  fprintf(stderr, "checking metadata\n");

  metadatum = dpl_dict_get(metadata_returned, "foo");
  if (NULL == metadatum)
    {
      fprintf(stderr, "missing metadatum\n");
      ret = 1;
      goto free_all;
    }

  assert(metadatum->val->type == DPL_VALUE_STRING);
  if (strcmp(metadatum->val->string, "bar"))
    {
      fprintf(stderr, "bad value in metadatum\n");
      ret = 1;
      goto free_all;
    }

  metadatum = dpl_dict_get(metadata_returned, "foo2");
  if (NULL == metadatum)
    {
      fprintf(stderr, "missing metadatum\n");
      ret = 1;
      goto free_all;
    }

  assert(metadatum->val->type == DPL_VALUE_STRING);
  if (strcmp(metadatum->val->string, "qux"))
    {
      fprintf(stderr, "bad value in metadatum\n");
      ret = 1;
      goto free_all;
    }

  /**/

  fprintf(stderr, "setting MD only\n");

  ret = dpl_dict_add(metadata, "foo", "bar2", 0);
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "error updating metadatum: %s (%d)\n", dpl_status_str(ret), ret);
      ret = 1;
      goto free_all;
    }

  ret = dpl_copy_id(ctx,           //the context
                    NULL,          //no src bucket
                    id,            //the key
                    0,
                    NULL,          //no subresource
                    NULL,          //no dst bucket
                    id,            //the same key
                    0,
                    NULL,          //no subresource
                    NULL,
                    DPL_FTYPE_REG, //object type
                    DPL_COPY_DIRECTIVE_METADATA_REPLACE,  //tell server to replace metadata
                    metadata,      //the updated metadata
                    NULL,          //no sysmd
                    NULL);         //no condition
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "error updating metadata: %s (%d)\n", dpl_status_str(ret), ret);
      ret = 1;
      goto free_all;
    }

  /**/

  fprintf(stderr, "getting MD only\n");

  ret = dpl_head_id(ctx,      //the context
                    NULL,     //no bucket,
                    id,       //the key
                    0,
                    NULL,     //no subresource
                    NULL,
                    NULL,     //no condition,
                    &metadata2_returned,
                    NULL);
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "error getting metadata: %s (%d)\n", dpl_status_str(ret), ret);
      ret = 1;
      goto free_all;
    }

  fprintf(stderr, "checking metadata\n");

  metadatum = dpl_dict_get(metadata2_returned, "foo");
  if (NULL == metadatum)
    {
      fprintf(stderr, "missing metadatum\n");
      ret = 1;
      goto free_all;
    }

  assert(metadatum->val->type == DPL_VALUE_STRING);
  if (strcmp(metadatum->val->string, "bar2"))
    {
      fprintf(stderr, "bad value in metadatum\n");
      ret = 1;
      goto free_all;
    }

  metadatum = dpl_dict_get(metadata2_returned, "foo2");
  if (NULL == metadatum)
    {
      fprintf(stderr, "missing metadatum\n");
      ret = 1;
      goto free_all;
    }

  assert(metadatum->val->type == DPL_VALUE_STRING);
  if (strcmp(metadatum->val->string, "qux"))
    {
      fprintf(stderr, "bad value in metadatum\n");
      ret = 1;
      goto free_all;
    }

  /**/

  fprintf(stderr, "delete object+MD\n");

  ret = dpl_delete_id(ctx,       //the context
                      NULL,      //no bucket
                      id,        //the key
                      0,         //enterprise number
                      NULL,      //no subresource
                      NULL,      //no option
                      NULL);     //no condition
  if (DPL_SUCCESS != ret)
    {
      fprintf(stderr, "error deleting object: %s (%d)\n", dpl_status_str(ret), ret);
      ret = 1;
      goto free_all;
    }
#endif

  ret = 0;

 free_all:

  if (NULL != metadata2_returned)
    dpl_dict_free(metadata2_returned);

  if (NULL != metadata_returned)
    dpl_dict_free(metadata_returned);

  if (NULL != data_buf_returned)
    free(data_buf_returned);

  if (NULL != metadata)
    dpl_dict_free(metadata);

  if (NULL != data_buf)
    free(data_buf);

  dpl_ctx_free(ctx); //free context

 free_dpl:
  dpl_free();        //free droplet library

 end:
  return ret;
}