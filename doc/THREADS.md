# GPU threads configuration

#### `index`
GPU index number usually starts from `0`.

#### `intensity`
Number of parallel GPU threads (nothing to do with CPU threads).

#### `worksize`
Number of local GPU threads (nothing to do with CPU threads), default value `8`.

#### `affine_to_cpu`
This will affine the thread to a CPU. This can make a GPU miner play along nicer with a CPU miner. Number or `false`, default value `false`.

#### `strided_index`
Switch memory pattern used for the scratchpad memory.

* `0` or `false` use a contiguous block of memory per thread.
* `1` or `true` use 16 byte contiguous memory per thread, the next memory block has offset of intensity blocks.
* `2` chunked memory, chunk size is controlled by `mem_chunk`, **intensity must be a multiple of worksize**.

For cryptonight variant 2 value `1` should never used, on NVIDIA platform only value `0` available.

#### `mem_chunk`
range `0` to `18`: set the number of elements (16 byte) per chunk. This value is only used if `strided_index` equal to `2`. Element count is computed with the equation: 2 to the power of `mem_chunk` e.g. 4 means a chunk of 16 elements (256 byte).

#### `comp_mode`
Compatibility enable/disable the automatic guard around compute kernel which allows to use a intensity which is not the multiple of the worksize. If you set `false` and the intensity is not multiple of the worksize the miner can crash, in this case set the `intensity` to a multiple of the `worksize` or activate `comp_mode`.

#### `unroll`
Allow to control how often the POW main loop is unrolled; valid range from 1 to 128 - for most OpenCL implementations it must be a power of two.

## Example

```json
    "threads": [
        {
            "index": 0,
            "intensity": 1000,
            "worksize": 8,
            "strided_index": 1,
            "mem_chunk": 2,
            "unroll": 8,
            "comp_mode": true,
            "affine_to_cpu": false
        }
    ],
```
