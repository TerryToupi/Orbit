# Static-mesh fixtures

These JSON documents are the JSON chunks of tiny GLBs. `asset_tests.cpp` constructs their binary chunks explicitly:
three positions, normals, UVs and indices, with tangent and uint32 source-index variants. Named multi-mesh fixtures test selection
and source reordering independently of instance transforms. Generated GLBs and modified variants stay in the test build directory.
No external asset repository or runtime-deployed test data is required.
