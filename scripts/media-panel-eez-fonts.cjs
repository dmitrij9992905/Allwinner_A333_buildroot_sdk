// Source-only export using the same lv_font_conv writers bundled with Studio.
// FreeType firmware reads layout metrics from these preview sources, but does
// not link their bitmap glyph tables or invoke this workstation tool.
const fs = require('fs');
const path = require('path');

async function main() {
    const [projectPath, resources] = process.argv.slice(2);
    const project = JSON.parse(fs.readFileSync(projectPath, 'utf8'));
    const build = project.settings.build;
    if (build.fontExportMode !== 'source') return;
    const converter = path.join(resources, 'app.asar', 'node_modules', 'lv_font_conv', 'lib');
    const collect = require(path.join(converter, 'collect_font_data'));
    const writeSource = require(path.join(converter, 'writers', 'lvgl'));
    const outputDirectory = path.resolve(path.dirname(projectPath), build.destinationFolder,
        build.separateFolderForImagesAndFonts ? 'fonts' : '');
    for (const font of project.fonts || []) {
        if (font.lvglUseFreeType) throw new Error('Expected source-only fonts, not FreeType');
        const name = 'ui_font_' + font.name.replace(/([a-z0-9])([A-Z])/g, '$1_$2')
            .replace(/[^a-zA-Z0-9_]/g, '_').toLowerCase();
        const output = path.join(outputDirectory, name + '.c');
        if (fs.existsSync(output) && fs.statSync(output).size) continue;
        const embedded = font.embeddedFontFile ||
            project.fonts[font.embeddedFontFileIndex]?.embeddedFontFile;
        const fontPath = path.resolve(path.dirname(projectPath), font.source.filePath);
        const range = [];
        for (const entry of (font.lvglRanges || '').split(',').filter(Boolean)) {
            const parts = entry.trim().split('=>');
            const limits = parts[0].split('-').map(Number);
            const first = limits[0], last = limits[1] ?? first;
            const mapped = parts[1] === undefined ? first : Number(parts[1]);
            if (![first, last, mapped].every(Number.isInteger) || last < first || first < 0)
                throw new Error('Invalid glyph range in ' + font.name);
            range.push(first, last, mapped);
        }
        if (font.lvglAdditionalSources?.length)
            throw new Error('Export additional font sources through Studio first');
        const args = {
            font: [{source_path: fontPath,
                source_bin: embedded ? Buffer.from(embedded, 'base64') : fs.readFileSync(fontPath),
                ranges: [{range, symbols: font.lvglSymbols || ''}]}],
            size: font.source.size, bpp: font.bpp, output: name,
            lv_include: build.lvglInclude, no_compress: true,
            lcd: false, lcd_v: false, use_color_info: false,
            no_kerning: false, no_prefilter: false, fast_kerning: false,
            lv_fallback: font.lvglFallbackFont || undefined,
            opts_string: 'EEZ Studio source font: ' + font.name
        };
        const data = await collect(args);
        fs.mkdirSync(outputDirectory, {recursive: true});
        fs.writeFileSync(output, writeSource(args, data)[name]);
        console.log('Font C source exported: ' + name);
    }
}
main().catch(error => { console.error(error.message); process.exitCode = 1; });
