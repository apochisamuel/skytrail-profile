# Skytrail Parsing Architecture

Skytrail Binary Profiles use a fixed header, a table directory, and typed sections. The implementation processes a profile in ordered stages so later stages can build on decoded records and indexes.

1. **Header** reads the format identity, version, directory location, and declared size.
2. **Directory** records section tags, byte spans, and section flags.
3. **Strings** decodes identifier-keyed text records used by later sections.
4. **Nodes** decodes primary geometric records and their payload data.
5. **Paths, grids, annotations, palettes, and links** decode secondary records that identify related primary records.
6. **Caches** build identifier lookup tables and compact query views.
7. **Resolution** connects record identifiers to decoded node, string, grid, and path records.
8. **Checksum processing** derives a profile-level integrity value from source bytes and directory data.
9. **Validation** evaluates cross-section profile relationships and selects usable record components.
10. **Cross-reference evaluation** derives final sequence values from resolved views.

The public query layer combines profile records into graph, timeline, annotation, palette, and aggregate analysis values. The source tree also contains dedicated analysis components that provide independent, deterministic summaries for callers and tools.
