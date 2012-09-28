/*
 * Copyright (C) 2010 SCALITY SA. All rights reserved.
 * http://www.scality.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SCALITY SA ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SCALITY SA OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of SCALITY SA.
 *
 * https://github.com/scality/Droplet
 */
#ifndef __DPL_ASYNC_H__
#define __DPL_ASYNC_H__ 1

typedef enum
  {
    TASK_LIST_ALL_MY_BUCKETS,
    TASK_LIST_BUCKET,
  } dpl_async_task_type_t;

typedef struct
{
  dpl_task_t task; /*!< mandatory */
  dpl_ctx_t *ctx;
  dpl_async_task_type_t type;
  dpl_task_func_t cb_func;
  void *cb_arg;
  dpl_status_t ret;
  union
  {
    struct
    {
      /* output */
      dpl_vec_t *buckets;
    } list_all_my_buckets;
    struct
    {
      /* input */
      char *bucket;
      char *prefix;
      char *delimiter;
      /* output */
      dpl_vec_t *objects;
      dpl_vec_t *common_prefixes;
    } list_bucket;
  } u;
} dpl_async_task_t;

void dpl_async_task_free(dpl_async_task_t *task);
dpl_task_t *dpl_list_all_my_buckets_async_prepare(dpl_ctx_t *ctx);
dpl_task_t *dpl_list_bucket_async_prepare(dpl_ctx_t *ctx, const char *bucket, const char *prefix, const char *delimiter);

#endif