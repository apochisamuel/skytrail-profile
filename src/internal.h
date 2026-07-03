#ifndef SKYTRAIL_INTERNAL_H
#define SKYTRAIL_INTERNAL_H

#include "skytrail.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SF_MAGIC_0 'S'
#define SF_MAGIC_1 'B'
#define SF_MAGIC_2 'R'
#define SF_MAGIC_3 'P'
#define SF_MAX_SECTIONS 128u
#define SF_MAX_RECORDS 65535u
#define SF_MAX_SECTION_BYTES (8u * 1024u * 1024u)
#define SF_MAX_TEXT_BYTES (2u * 1024u * 1024u)
#define SF_TAG(a,b,c,d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24))
#define SF_TAG_STRS SF_TAG('S','T','R','S')
#define SF_TAG_NODE SF_TAG('N','O','D','E')
#define SF_TAG_PATH SF_TAG('P','A','T','H')
#define SF_TAG_GRID SF_TAG('G','R','I','D')
#define SF_TAG_META SF_TAG('M','E','T','A')
#define SF_TAG_PALC SF_TAG('P','A','L','C')
#define SF_TAG_LINK SF_TAG('L','I','N','K')
#define SF_TAG_STAT SF_TAG('S','T','A','T')

typedef struct SfReader {
    const uint8_t *data;
    size_t size;
    size_t pos;
} SfReader;

typedef struct SfHeader {
    uint16_t version;
    uint16_t section_count;
    uint32_t directory_offset;
    uint32_t declared_size;
} SfHeader;

typedef struct SfDirectoryEntry {
    uint32_t tag;
    uint32_t offset;
    uint32_t length;
    uint32_t flags;
} SfDirectoryEntry;

typedef struct SfString {
    uint16_t id;
    uint8_t group;
    uint8_t flags;
    uint8_t state;
    size_t length;
    char *text;
} SfString;

typedef struct SfNode {
    uint16_t id;
    uint16_t title_id;
    uint8_t kind;
    uint8_t flags;
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
    uint8_t state;
    size_t payload_len;
    uint8_t *payload;
    const char *title;
} SfNode;

typedef struct SfPath {
    uint16_t id;
    uint16_t node_id;
    uint16_t name_id;
    uint8_t flags;
    uint8_t state;
    size_t sample_count;
    size_t cache_sample_count;
    uint16_t *samples;
    SfNode *node;
    const char *name;
    const uint8_t *payload_view;
    size_t payload_view_len;
} SfPath;

typedef struct SfGrid {
    uint16_t id;
    uint16_t label_id;
    uint8_t flags;
    uint8_t state;
    size_t member_count;
    size_t cache_member_count;
    uint16_t *members;
    SfNode **node_links;
    const char *label;
} SfGrid;

typedef struct SfAnnotation {
    uint16_t id;
    uint16_t key_id;
    uint16_t node_id;
    uint8_t flags;
    uint8_t state;
    size_t value_len;
    char *value;
    const char *key;
    SfNode *node;
} SfAnnotation;

typedef struct SfPalette {
    uint16_t id;
    uint16_t name_id;
    uint16_t node_id;
    uint8_t flags;
    uint8_t state;
    size_t tone_count;
    size_t cache_tone_count;
    uint16_t *tones;
    const char *name;
    SfNode *node;
} SfPalette;

typedef struct SfLink {
    uint16_t source_grid;
    uint16_t target_path;
    uint16_t note_id;
    uint8_t mode;
    uint8_t state;
    SfGrid *grid;
    SfPath *path;
    const char *note;
} SfLink;

typedef struct SfCaches {
    const char **names_by_id;
    size_t names_cap;
    SfNode **nodes_by_id;
    size_t nodes_cap;
    const uint8_t **payloads_by_path;
    size_t payloads_cap;
    const uint16_t **samples_by_path;
    size_t samples_cap;
    const uint16_t **members_by_grid;
    size_t members_cap;
    const char **values_by_annotation;
    size_t values_cap;
    const uint16_t **tones_by_palette;
    size_t tones_cap;
    uint64_t string_hash;
    uint64_t node_hash;
    uint64_t topology_hash;
} SfCaches;

struct SfProfile {
    SfStatus status;
    char error[160];
    const uint8_t *source;
    size_t source_size;
    SfHeader header;
    SfDirectoryEntry *directory;
    size_t directory_count;
    SfString *strings;
    size_t string_count;
    SfNode *nodes;
    size_t node_count;
    SfPath *paths;
    size_t path_count;
    SfGrid *grids;
    size_t grid_count;
    SfAnnotation *annotations;
    size_t annotation_count;
    SfPalette *palettes;
    size_t palette_count;
    SfLink *links;
    size_t link_count;
    SfCaches caches;
    uint64_t checksum_value;
    uint64_t validation_value;
    uint64_t sequence_value;
};

void *sf_allocate(size_t count, size_t size);
void *sf_resize(void *ptr, size_t count, size_t size);
char *sf_copy_text(const uint8_t *bytes, size_t length);
void sf_set_error(SfProfile *profile, SfStatus status, const char *message);
void sf_clear_error(SfProfile *profile);

bool sf_reader_init(SfReader *reader, const uint8_t *data, size_t size);
bool sf_reader_remaining(const SfReader *reader, size_t count);
bool sf_reader_u8(SfReader *reader, uint8_t *out);
bool sf_reader_u16(SfReader *reader, uint16_t *out);
bool sf_reader_i16(SfReader *reader, int16_t *out);
bool sf_reader_u32(SfReader *reader, uint32_t *out);
bool sf_reader_bytes(SfReader *reader, const uint8_t **out, size_t count);
bool sf_reader_skip(SfReader *reader, size_t count);
bool sf_reader_slice(const uint8_t *data, size_t size, size_t offset, size_t length, SfReader *out);

uint64_t sf_mix64(uint64_t value);
uint64_t sf_hash_bytes(const uint8_t *data, size_t length, uint64_t seed);
uint64_t sf_hash_text(const char *text, uint64_t seed);
uint32_t sf_tag_from_bytes(const uint8_t bytes[4]);
size_t sf_next_capacity(size_t current, size_t needed);

SfStatus sf_parse_header(SfProfile *profile);
SfStatus sf_parse_directory(SfProfile *profile);
const SfDirectoryEntry *sf_find_section(const SfProfile *profile, uint32_t tag);
SfStatus sf_section_reader(const SfProfile *profile, uint32_t tag, SfReader *reader);
SfStatus sf_parse_strings(SfProfile *profile);
SfStatus sf_parse_nodes(SfProfile *profile);
SfStatus sf_parse_paths(SfProfile *profile);
SfStatus sf_parse_grids(SfProfile *profile);
SfStatus sf_parse_annotations(SfProfile *profile);
SfStatus sf_parse_palettes(SfProfile *profile);
SfStatus sf_parse_links(SfProfile *profile);
SfStatus sf_build_caches(SfProfile *profile);
SfStatus sf_resolve_references(SfProfile *profile);
SfStatus sf_verify_checksum(SfProfile *profile);
SfStatus sf_validate_profile(SfProfile *profile);
SfStatus sf_crossref_run(SfProfile *profile);

SfString *sf_find_string_record(SfProfile *profile, uint16_t id);
SfNode *sf_find_node_record(SfProfile *profile, uint16_t id);
SfPath *sf_find_path_record(SfProfile *profile, uint16_t id);
SfGrid *sf_find_grid_record(SfProfile *profile, uint16_t id);
const char *sf_cache_name(const SfProfile *profile, uint16_t id);
SfNode *sf_cache_node(const SfProfile *profile, uint16_t id);
void sf_release_string_text(SfString *item);
void sf_release_node_payload(SfNode *item);
void sf_release_path_samples(SfPath *item);
void sf_release_grid_members(SfGrid *item);
void sf_release_annotation_value(SfAnnotation *item);
void sf_release_palette_tones(SfPalette *item);

uint64_t sf_metric_nodes(const SfProfile *profile);
uint64_t sf_metric_paths(const SfProfile *profile);
uint64_t sf_metric_grids(const SfProfile *profile);
uint64_t sf_metric_annotations(const SfProfile *profile);
uint64_t sf_metric_palettes(const SfProfile *profile);
uint64_t sf_metric_strings(const SfProfile *profile);
uint64_t sf_metric_links(const SfProfile *profile);
uint64_t sf_metric_geometry(const SfProfile *profile);
uint64_t sf_metric_timeline(const SfProfile *profile);
uint64_t sf_metric_distribution(const SfProfile *profile);
uint64_t sf_metric_density(const SfProfile *profile);
uint64_t sf_metric_symmetry(const SfProfile *profile);
uint64_t sf_metric_signature(const SfProfile *profile);
uint64_t sf_metric_clusters(const SfProfile *profile);
uint64_t sf_metric_horizon(const SfProfile *profile);
uint64_t sf_metric_coverage(const SfProfile *profile);
uint64_t sf_metric_coherence(const SfProfile *profile);
uint64_t sf_metric_entropy(const SfProfile *profile);
uint64_t sf_metric_balance(const SfProfile *profile);
uint64_t sf_metric_span(const SfProfile *profile);
uint64_t sf_metric_route_quality(const SfProfile *profile);
uint64_t sf_metric_palette_range(const SfProfile *profile);
uint64_t sf_metric_annotation_shape(const SfProfile *profile);
uint64_t sf_metric_grid_shape(const SfProfile *profile);
uint64_t sf_metric_section_layout(const SfProfile *profile);
uint64_t sf_metric_profile_shape(const SfProfile *profile);
uint64_t sf_layer_anchor(const SfProfile *profile);
uint64_t sf_layer_aperture(const SfProfile *profile);
uint64_t sf_layer_balance(const SfProfile *profile);
uint64_t sf_layer_bands(const SfProfile *profile);
uint64_t sf_layer_beacon(const SfProfile *profile);
uint64_t sf_layer_border(const SfProfile *profile);
uint64_t sf_layer_cadence(const SfProfile *profile);
uint64_t sf_layer_channel(const SfProfile *profile);
uint64_t sf_layer_contour(const SfProfile *profile);
uint64_t sf_layer_delta(const SfProfile *profile);
uint64_t sf_layer_drift(const SfProfile *profile);
uint64_t sf_layer_edge(const SfProfile *profile);
uint64_t sf_layer_field(const SfProfile *profile);
uint64_t sf_layer_frame(const SfProfile *profile);
uint64_t sf_layer_gate(const SfProfile *profile);
uint64_t sf_layer_gridline(const SfProfile *profile);
uint64_t sf_layer_harmonic(const SfProfile *profile);
uint64_t sf_layer_horizon(const SfProfile *profile);
uint64_t sf_layer_interval(const SfProfile *profile);
uint64_t sf_layer_junction(const SfProfile *profile);
uint64_t sf_layer_kernel(const SfProfile *profile);
uint64_t sf_layer_lattice(const SfProfile *profile);
uint64_t sf_layer_marker(const SfProfile *profile);
uint64_t sf_layer_mesh(const SfProfile *profile);
uint64_t sf_layer_notch(const SfProfile *profile);
uint64_t sf_layer_orbit(const SfProfile *profile);
uint64_t sf_layer_pivot(const SfProfile *profile);
uint64_t sf_layer_quadrant(const SfProfile *profile);
uint64_t sf_layer_ribbon(const SfProfile *profile);
uint64_t sf_layer_span(const SfProfile *profile);
uint64_t sf_layer_trace(const SfProfile *profile);
uint64_t sf_layer_uplink(const SfProfile *profile);
uint64_t sf_layer_vector(const SfProfile *profile);
uint64_t sf_layer_window(const SfProfile *profile);
uint64_t sf_layer_xray(const SfProfile *profile);
uint64_t sf_layer_yield(const SfProfile *profile);
uint64_t sf_layer_zone(const SfProfile *profile);
uint64_t sf_layer_arc(const SfProfile *profile);
uint64_t sf_layer_basin(const SfProfile *profile);
uint64_t sf_layer_crown(const SfProfile *profile);
uint64_t sf_layer_dome(const SfProfile *profile);
uint64_t sf_layer_ember(const SfProfile *profile);
uint64_t sf_layer_facet(const SfProfile *profile);
uint64_t sf_layer_glint(const SfProfile *profile);
uint64_t sf_layer_halo(const SfProfile *profile);
uint64_t sf_layer_inlet(const SfProfile *profile);
uint64_t sf_layer_jet(const SfProfile *profile);
uint64_t sf_layer_knot(const SfProfile *profile);
uint64_t sf_layer_lens(const SfProfile *profile);
uint64_t sf_layer_meridian(const SfProfile *profile);
uint64_t sf_layer_nexus(const SfProfile *profile);
uint64_t sf_layer_offset(const SfProfile *profile);
uint64_t sf_layer_prism(const SfProfile *profile);
uint64_t sf_layer_quiver(const SfProfile *profile);
uint64_t sf_layer_ridge(const SfProfile *profile);
uint64_t sf_layer_shard(const SfProfile *profile);
uint64_t sf_layer_thread(const SfProfile *profile);
uint64_t sf_layer_umbra(const SfProfile *profile);
uint64_t sf_layer_verge(const SfProfile *profile);
uint64_t sf_layer_weave(const SfProfile *profile);
uint64_t sf_layer_axis(const SfProfile *profile);
uint64_t sf_layer_bloom(const SfProfile *profile);
uint64_t sf_layer_chord(const SfProfile *profile);
uint64_t sf_layer_dais(const SfProfile *profile);
uint64_t sf_layer_focal(const SfProfile *profile);
uint64_t sf_layer_gauge(const SfProfile *profile);
uint64_t sf_layer_helix(const SfProfile *profile);
uint64_t sf_layer_island(const SfProfile *profile);
uint64_t sf_layer_jewel(const SfProfile *profile);
uint64_t sf_layer_keystone(const SfProfile *profile);
uint64_t sf_analysis_total(const SfProfile *profile);
uint64_t sf_component_catalog(const SfProfile *profile);
uint64_t sf_component_coordinate(const SfProfile *profile);
uint64_t sf_component_cumulative(const SfProfile *profile);
uint64_t sf_component_descriptor(const SfProfile *profile);
uint64_t sf_component_envelope(const SfProfile *profile);
uint64_t sf_component_footprint(const SfProfile *profile);
uint64_t sf_component_gradient(const SfProfile *profile);
uint64_t sf_component_histogram(const SfProfile *profile);
uint64_t sf_component_identity(const SfProfile *profile);
uint64_t sf_component_ledger(const SfProfile *profile);
uint64_t sf_component_manifest(const SfProfile *profile);
uint64_t sf_component_navigator(const SfProfile *profile);
uint64_t sf_component_observer(const SfProfile *profile);
uint64_t sf_component_partition(const SfProfile *profile);
uint64_t sf_component_quality(const SfProfile *profile);
uint64_t sf_component_raster(const SfProfile *profile);
uint64_t sf_component_scheduler(const SfProfile *profile);
uint64_t sf_component_topology(const SfProfile *profile);
uint64_t sf_component_unifier(const SfProfile *profile);
uint64_t sf_component_viewport(const SfProfile *profile);
uint64_t sf_component_waypoint(const SfProfile *profile);
uint64_t sf_component_xsection(const SfProfile *profile);
uint64_t sf_component_yardstick(const SfProfile *profile);
uint64_t sf_component_zenith(const SfProfile *profile);
uint64_t sf_component_assembler(const SfProfile *profile);
uint64_t sf_component_composer(const SfProfile *profile);
uint64_t sf_component_decoder(const SfProfile *profile);
uint64_t sf_component_evaluator(const SfProfile *profile);
uint64_t sf_component_formatter(const SfProfile *profile);
uint64_t sf_component_generator(const SfProfile *profile);
uint64_t sf_component_heuristic(const SfProfile *profile);
uint64_t sf_component_inspector(const SfProfile *profile);
uint64_t sf_component_joiner(const SfProfile *profile);
uint64_t sf_component_kinetics(const SfProfile *profile);
uint64_t sf_component_ledger_map(const SfProfile *profile);
uint64_t sf_component_matrix(const SfProfile *profile);
uint64_t sf_component_notebook(const SfProfile *profile);
uint64_t sf_component_operator(const SfProfile *profile);
uint64_t sf_component_projection(const SfProfile *profile);
uint64_t sf_component_quorum(const SfProfile *profile);
uint64_t sf_component_total(const SfProfile *profile);

#endif
