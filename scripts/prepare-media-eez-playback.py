#!/usr/bin/env python3
"""Print an apply_patch migration for the authored EEZ project; never write it."""
import copy
import difflib
import json
from pathlib import Path
import sys
import uuid


def renew_ids(value):
    if isinstance(value, dict):
        for key, child in value.items():
            if key == 'objID':
                value[key] = str(uuid.uuid4())
            else:
                renew_ids(child)
    elif isinstance(value, list):
        for child in value:
            renew_ids(child)


def widgets(items):
    for item in items:
        yield item
        yield from widgets(item.get('children', []))


def migrate(project):
    variables = project['variables']['globalVariables']
    existing = {v['name']: v for v in variables}
    for name, kind, default in [('track_position_duration', 'string', '"00:00 / 00:00"'),
                                ('track_progress', 'integer', '0'),
                                ('track_seekable', 'boolean', 'false'),
                                ('usb_available', 'boolean', 'false'),
                                ('sd_available', 'boolean', 'false')]:
        if name not in existing:
            value = dict(objID=str(uuid.uuid4()), name=name, type=kind,
                         defaultValue=default, persistent=False, native=True)
            variables.append(value)
        else:
            existing[name]['native'] = True
    actions = project['actions']
    for name in ('seek', 'usb', 'sd'):
        if not any(a['name'] == name for a in actions):
            action = copy.deepcopy(next(a for a in actions if a['name'] == 'music'))
            renew_ids(action)
            action['name'] = name
            actions.append(action)
    main = next(p for p in project['userPages'] if p['name'] == 'Main')
    all_widgets = list(widgets(main['components']))
    bar = next(w for w in all_widgets if w.get('identifier') == 'track_progress_slider' or
               (w['type'] == 'LVGLBarWidget' and not w.get('identifier')))
    bar.update(type='LVGLSliderWidget', identifier='track_progress_slider',
               min=0, max=10000, value='track_progress', valueType='expression',
               disabledState='!track_seekable', disabledStateType='expression',
               clickableFlag=True, clickableFlagType='literal')
    if not any(e.get('eventName') == 'RELEASED' for e in bar.get('eventHandlers', [])):
        bar.setdefault('eventHandlers', []).append(dict(objID=str(uuid.uuid4()),
            eventName='RELEASED', handlerType='flow', userData=0))
    bar['localStyles'].setdefault('definition', {}).setdefault('KNOB', {})['DEFAULT'] = {
        'bg_color': '#C9C9C9', 'bg_opa': 255, 'radius': 20, 'pad_all': 6}
    cover = next(w for w in all_widgets if w.get('identifier') == 'album_cover_image')
    cover.update(widthUnit='px', heightUnit='px', width=360, height=360,
                 innerAlign='CONTAIN')
    source = next(w for w in all_widgets if w.get('identifier') == 'source_tab')
    music = next(w for w in all_widgets if w.get('identifier') == 'select_music')
    help_text = next(w for w in all_widgets if w.get('identifier') == 'sources_help')
    help_text['text'] = 'Локальная коллекция, USB, SD, интернет-радио или Bluetooth'
    for button in all_widgets:
        if button.get('identifier') in ('select_music', 'select_radio', 'select_bluetooth'):
            button['top'] = 359
    for name, caption, left in [('usb', 'USB-накопители', 225), ('sd', 'Карта памяти SD', 665)]:
        button = next((w for w in all_widgets if w.get('identifier') == 'select_' + name), None)
        if button is None:
            button = copy.deepcopy(music)
            renew_ids(button)
            source['children'].append(button)
            add_call(main, button, name, 'CLICKED')
        button.update(identifier='select_' + name, left=left, top=465,
                      disabledState='!' + name + '_available', disabledStateType='expression')
        button['children'][0].update(identifier='select_' + name + '_caption', text=caption)
    # The authored time label was on the root, thus visible above every tab.
    # Keep it alongside the slider, within the playback tab's coordinate space.
    label = next(w for w in all_widgets if w.get('identifier') == 'track_position_duration_label')
    playing = next(w for w in all_widgets if w.get('identifier') == 'now_playing_tab')
    if label not in playing['children']:
        for parent in all_widgets:
            if label in parent.get('children', []):
                parent['children'].remove(label)
                break
        playing['children'].append(label)
    label.update(left=0, top=-27, width=160, height=29, widthUnit='px', heightUnit='px')
    label['localStyles'].setdefault('definition', {}).setdefault('MAIN', {}).setdefault('DEFAULT', {}).update(
        align='BOTTOM_RIGHT', text_align='RIGHT')
    if not any(c['source'] == bar['objID'] and c['output'] == 'RELEASED'
               for c in main['connectionLines']):
        add_call(main, bar, 'seek', 'RELEASED')
    return project


def add_call(page, widget, action, event):
    component = dict(objID=str(uuid.uuid4()), type='CallActionActionComponent',
                     left=1550, top=850 + len(page['components']) * 30, width=87,
                     height=24, action=action, userPropertyValues={
                         'objID': str(uuid.uuid4()), 'values': {}})
    page['components'].append(component)
    page['connectionLines'].append(dict(objID=str(uuid.uuid4()), source=widget['objID'],
                                        output=event, target=component['objID'], input='@seqin'))


if __name__ == '__main__':
    path = Path(sys.argv[1]).resolve()
    old = path.read_text()
    new = json.dumps(migrate(json.loads(old)), ensure_ascii=False, indent=2) + '\n'
    diff = list(difflib.unified_diff(old.splitlines(True), new.splitlines(True)))
    if diff:
        print('*** Begin Patch\n*** Update File: ' + str(path))
        # apply_patch derives line positions from context; omit unified offsets.
        for line in diff[2:]:
            print('@@' if line.startswith('@@') else line.rstrip('\n'))
        print('*** End Patch')
