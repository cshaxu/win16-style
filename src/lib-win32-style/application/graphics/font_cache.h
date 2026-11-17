#ifndef STYLE_GRAPHICS_FONT_CACHE_H
#define STYLE_GRAPHICS_FONT_CACHE_H
#include <windows.h>

/* Operations keep the cache alive and do not reenter that same cache.
 * release accepts NULL and borrowed native fonts; retain is required for
 * snapshot replacements distinct from their source. No operation reports UI. */
typedef struct style_font_ops {
    HFONT (*acquire)(HFONT source);
    BOOL (*retain)(HFONT);
    void (*release)(HFONT);
} style_font_ops;
typedef struct style_font_glyph_range {
    UINT code_page;
    BYTE first;
    unsigned count;
} style_font_glyph_range;

/* Zero-initialized GUI-thread-owned cache. Source handles are borrowed;
 * replacement leases and lazy glyph maps are owned until dispose. Use the
 * same immutable ops and range throughout a cache lifetime. The existing
 * finite capacities are 32 source fonts and 224 single-byte glyphs per map. */
typedef struct style_font_cache {
    HFONT source[32],font[32];
    struct style_font_glyph_map *glyphs[32];
    unsigned count;
    const style_font_ops *ops;
} style_font_cache;
HFONT style_font_cache_get(style_font_cache *,HFONT,const style_font_ops *);
BOOL style_font_cache_decode(style_font_cache *,HDC,const WCHAR *,UINT,WCHAR *,style_font_glyph_range);
void style_font_cache_dispose(style_font_cache *);

/* One source signature: native LOGFONT/metrics and ASCII32..127 advances.
 * A changed signature invalidates the cached replacement. Active borrowers
 * block replacement; return releases their lease before deferred invalidation.
 * As in the original consumer, a failed acquire drops the obsolete cached
 * font rather than keeping it. ops and enclosing storage outlive all borrows. */
typedef struct style_font_snapshot_cache {
    HFONT font,source;
    LOGFONTW log;
    TEXTMETRICW metrics;
    int widths[96];
    unsigned borrows;
    BOOL invalid;
    const style_font_ops *ops;
} style_font_snapshot_cache;
HFONT style_font_snapshot_borrow(style_font_snapshot_cache *,HDC,void *owner,void **token,const style_font_ops *);
void style_font_snapshot_return(style_font_snapshot_cache *,HFONT);
/* Dispose only after borrowers have restored their DC and returned leases. */
void style_font_snapshot_dispose(style_font_snapshot_cache *);

/* A published native font and optional AddFontMemResourceEx registration.
 * The factory cleans unpublished resources and commits only on success;
 * acquire/release own the reference count and last-reference GDI retirement. */
typedef struct style_memory_font {
    HFONT font;
    HANDLE resource;
    unsigned references;
} style_memory_font;
HFONT style_memory_font_acquire(style_memory_font *,BOOL (*create)(void *,style_memory_font *),void *);
void style_memory_font_release(style_memory_font *,HFONT);

typedef struct style_font_pool_entry {
    struct style_font_pool_entry *next;
    HFONT source;
    UINT key;
    BOOL isolated;
    style_memory_font lease;
} style_font_pool_entry;
typedef struct style_font_pool {style_font_pool_entry *head;} style_font_pool;
/* A GUI-thread-owned pool with caller-defined recipe keys. The factory owns
 * its temporary entry; transfer it on success via owned, or leave NULL for a
 * borrowed native font. This preserves existing factory allocation order.
 * Balance every lease; there is no forced teardown of outstanding borrows. */
typedef HFONT (*style_font_pool_factory)(HFONT,UINT,void *,style_font_pool_entry **owned);
HFONT style_font_pool_get(style_font_pool *,HFONT,UINT,BOOL isolated,style_font_pool_factory,void *);
HFONT style_font_pool_source(const style_font_pool *,HFONT);
BOOL style_font_pool_try_retain(style_font_pool *,HFONT);
void style_font_pool_retain(style_font_pool *,HFONT);
void style_font_pool_release(style_font_pool *,HFONT);
/* Zero-initialized synchronous temporary translation; caller supplies a
 * prevalidated run and single-byte encoding/decoder policy. On failure any
 * partially allocated buffers remain owned by this root until end. Native
 * text/options are published by caller only after success. Normal return
 * only; no new exception cleanup contract. Do not prepare again before end. */
typedef struct style_glyph_text {WCHAR *decoded;char *encoded;} style_glyph_text;
BOOL style_glyph_text_prepare(style_glyph_text *,HDC,LPCWSTR,UINT count,UINT code_page,BOOL (*decode)(HDC,const WCHAR *,UINT,WCHAR *));
void style_glyph_text_release_ansi(style_glyph_text *);
void style_glyph_text_end(style_glyph_text *);
#endif
