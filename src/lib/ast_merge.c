/* vi:set ts=4 sw=4 expandtab:
 *
 * Copyright 2016, Chris Leishman (http://github.com/cleishm)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../../config.h"
#include "astnode.h"
#include "util.h"
#include <assert.h>


struct merge
{
    cypher_astnode_t _astnode;
    const cypher_astnode_t *path;
    unsigned int nactions;
    const cypher_astnode_t *actions[];
};


static ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size);


static const struct cypher_astnode_vt *parents[] =
    { &cypher_query_clause_astnode_vt };

const struct cypher_astnode_vt cypher_merge_astnode_vt =
    { .parents = parents,
      .nparents = 1,
      .name = "MERGE",
      .detailstr = detailstr,
      .free = cypher_astnode_free };


cypher_astnode_t *cypher_ast_merge(const cypher_astnode_t *path,
        cypher_astnode_t * const *actions, unsigned int nactions,
        cypher_astnode_t **children, unsigned int nchildren,
        struct cypher_input_range range)
{
    REQUIRE_TYPE(path, CYPHER_AST_PATTERN_PATH, NULL);
    REQUIRE_TYPE_ALL(actions, nactions, CYPHER_AST_MERGE_ACTION, NULL);

    struct merge *node = calloc(1, sizeof(struct merge) +
            nactions * sizeof(cypher_astnode_t *));
    if (node == NULL)
    {
        return NULL;
    }
    if (cypher_astnode_init(&(node->_astnode), CYPHER_AST_MERGE,
            children, nchildren, range))
    {
        goto cleanup;
    }
    node->path = path;
    memcpy(node->actions, actions, nactions * sizeof(cypher_astnode_t *));
    node->nactions = nactions;
    return &(node->_astnode);

    int errsv;
cleanup:
    errsv = errno;
    free(node);
    errno = errsv;
    return NULL;
}


ssize_t detailstr(const cypher_astnode_t *self, char *str, size_t size)
{
    REQUIRE_TYPE(self, CYPHER_AST_MERGE, -1);
    struct merge *node = container_of(self, struct merge, _astnode);

    size_t n = 0;
    ssize_t r = snprintf(str, size, "path=@%d", node->path->ordinal);
    if (r < 0)
    {
        return -1;
    }
    n += r;

    if (node->nactions > 0)
    {
        strncpy(str + n, ", actions=", (n < size)? size-n : 0);
        if (size > 0)
        {
            str[size-1] = '\0';
        }
        n += 8;

        r = snprint_sequence(str+n, (n < size)? size-n : 0,
                node->actions, node->nactions);
        if (r < 0)
        {
            return -1;
        }
        n += r;
    }
    return n;
}