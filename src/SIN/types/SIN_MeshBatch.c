#include "SIN_MeshBatch.h"

#include "lyarr/ZJC_Stack.h"
#include "lyutils/ZJC_Evil.h"
#include "GL/glew.h"

#include <stdio.h>

#define SIN_BATCH_VERTSIZE            56
#define SIN_MAX_BATCHES               64

#define SIN_BATCH_MAXOBJECTS          256
#define SIN_BATCH_MAXVERTS            65536

static M3DB    SIN_emptybatch       = {0};
static ushort  SIN_ACTIVE_BATCHES   = 0;
static ushort  __curBatchLoc        = 0;

static sStack* SIN_BATCH_SLOTSTACK  = NULL;

static M3DB*   SIN_meshbatches      = NULL;

#define SIN_BATCH_VBOSIZE SIN_BATCH_MAXVERTS * SIN_BATCH_VERTSIZE
#define SIN_BATCH_IBOSIZE SIN_BATCH_MAXVERTS * sizeof(uint)

M3DB*   SIN_active_meshbatch        = NULL;

//  - --- - --- - --- - --- -

void  SIN_Batcher_init()                        { 

    SIN_meshbatches     = (M3DB*) evil_malloc(SIN_MAX_BATCHES, sizeof(M3DB));
    SIN_BATCH_SLOTSTACK = build_sStack(SIN_MAX_BATCHES);

    for(int i = SIN_MAX_BATCHES-1;
        i > 0; i--)                             { sStack_push(SIN_BATCH_SLOTSTACK, i);                                  }
                                                                                                                        }

int  SIN_Batcher_end()                          {

    for(uint i = 0;
        i < SIN_ACTIVE_BATCHES; i ++)           { M3DB* batch = SIN_meshbatches + i;

                                                  if(batch->BUFFS[0])
                                                { SIN_delMeshBatch(SIN_meshbatches+i); }                                }

    del_sStack(SIN_BATCH_SLOTSTACK);

    return 0;                                                                                                           }

//  - --- - --- - --- - --- -

void  chkbatch     (ushort loc)                 { if(loc+1 != __curBatchLoc)
                                                { __curBatchLoc = loc+1; SIN_bindMeshBatch(loc); }                      }

void  resbatch     ()                           { __curBatchLoc = -1; SIN_active_meshbatch = NULL;                      }

void  SIN_bindMeshBatch(ushort loc)             { glBindVertexArray((SIN_meshbatches + loc)->VAO);
                                                  SIN_active_meshbatch = SIN_meshbatches + loc;                         }

//  - --- - --- - --- - --- -

M3DB* SIN_meshbatches_get  (ushort loc)         {

    if(loc < SIN_MAX_BATCHES)
    { 
        if(SIN_meshbatches+loc)                 { return SIN_meshbatches+loc;                                           }

        printf("Location %u points to an empty meshbatch slot\n", loc);
        return NULL;
    }

    printf("No meshbatch at location %u\n", loc);
    return NULL;                                                                                                        }

//  - --- - --- - --- - --- -

M3DB* SIN_genMeshBatch()                        {

    uint loc = sStack_pop(SIN_BATCH_SLOTSTACK);
    WARD_EVIL_UNSIG(loc, 1);

    M3DB* batch    = SIN_meshbatches+loc;

    batch->loc     = loc;
    batch->vcursor = 0;
    batch->icursor = 0;

    glGenVertexArrays(1, &batch->VAO);
    glBindVertexArray(batch->VAO);

    glGenBuffers(SIN_BATCH_NUM_BUFFERS, batch->BUFFS);
    glBindBuffer(GL_ARRAY_BUFFER, batch->BUFFS[SIN_BATCH_VBO]);
    glBufferData(GL_ARRAY_BUFFER, SIN_BATCH_VBOSIZE, NULL, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, SIN_BATCH_VERTSIZE, (void*) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, SIN_BATCH_VERTSIZE, (void*) (3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, SIN_BATCH_VERTSIZE, (void*) (6 * sizeof(float)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, SIN_BATCH_VERTSIZE, (void*) (9 * sizeof(float)));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, SIN_BATCH_VERTSIZE, (void*) (12 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->BUFFS[SIN_BATCH_IBO]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, SIN_BATCH_IBOSIZE, NULL, GL_STATIC_DRAW);

    glBindVertexArray(0);

    SIN_active_meshbatch = batch;

    return batch;                                                                                                       }

//  - --- - --- - --- - --- -

void SIN_delMeshBatch(M3DB* batch)              {

    for (uint i = 0;
         i < SIN_BATCH_NUM_BUFFERS; i++)        { if (batch->BUFFS[i])
                                                { glDeleteBuffers(1, &batch->BUFFS[i]); }                               }

    glDeleteVertexArrays(1, &batch->VAO);

    int memward = sStack_push(SIN_BATCH_SLOTSTACK, batch->loc);
    WARD_EVIL_UNSIG(memward, 1);

    SIN_meshbatches[batch->loc] = SIN_emptybatch;                                                                       }

//  - --- - --- - --- - --- -

void  SIN_meshbatch_upload(ushort  numVerts,
                           ushort  numIndices,
                           float*  vertex_data,
                           ushort* indices)     {

    uint  vsize = numVerts   * sizeof(float) * 14;
    uint  isize = numIndices * sizeof(uint)  * 3;

    for(ushort i = 0;
        i < numIndices * 3; i++ )               { indices[i] += SIN_active_meshbatch->vert_total;                      }

    SIN_active_meshbatch->vert_total += numVerts;

    glBindVertexArray(SIN_active_meshbatch->VAO);

    glBindBuffer     (GL_ARRAY_BUFFER, SIN_active_meshbatch->BUFFS[SIN_BATCH_VBO]);
    glBufferSubData  (GL_ARRAY_BUFFER, SIN_active_meshbatch->vcursor, vsize, (void*) vertex_data);

    glBindBuffer     (GL_ELEMENT_ARRAY_BUFFER, SIN_active_meshbatch->BUFFS[SIN_BATCH_IBO]);
    glBufferSubData  (GL_ELEMENT_ARRAY_BUFFER, SIN_active_meshbatch->icursor, isize, (void*) indices);

    // glBindVertexArray(0);

    SIN_active_meshbatch->vcursor += vsize;
    SIN_active_meshbatch->icursor += isize;                                                                             }

