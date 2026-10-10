#!/usr/bin/env python3
"""Emit an apply_patch migration, reusing the user's existing EEZ notify_panel."""
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


def migrate(project):
    variables = project['variables']['globalVariables']
    if not any(v['name'] == 'notify_visible' for v in variables):
        variables.append(dict(objID=str(uuid.uuid4()), name='notify_visible',
                              type='boolean', defaultValue='false',
                              persistent=False, native=True))
    assert any(w['name'] == 'notify_panel' for w in project['userWidgets'])
    for page, prefix in zip(project['userPages'], ('main', 'player', 'wifi')):
        root = page['components'][0]
        volume = next(w for w in root['children'] if w.get('userWidgetPageName') == 'notify_panel_volume')
        notice = next((w for w in root['children'] if w.get('userWidgetPageName') == 'notify_panel'), None)
        if notice is None:
            notice = copy.deepcopy(volume)
            renew_ids(notice)
            root['children'].append(notice)
        notice.update(identifier=prefix + '_notification_osd',
                      userWidgetPageName='notify_panel', left=0, top=0,
                      hiddenFlag='!notify_visible', hiddenFlagType='expression',
                      clickableFlag=False, clickableFlagType='literal', widgetFlags='')
        # Both OSDs occupy the top bar. The native notify_visible binding
        # hides ordinary notices while the higher-priority volume OSD is visible.
        volume['top'] = 0
    return project


if __name__ == '__main__':
    path = Path(sys.argv[1]).resolve()
    old = path.read_text()
    new = json.dumps(migrate(json.loads(old)), ensure_ascii=False, indent=2) + '\n'
    diff = list(difflib.unified_diff(old.splitlines(True), new.splitlines(True), n=45))
    if diff:
        print('*** Begin Patch\n*** Update File: ' + str(path))
        for line in diff[2:]:
            print('@@' if line.startswith('@@') else line.rstrip('\n'))
        print('*** End Patch')
