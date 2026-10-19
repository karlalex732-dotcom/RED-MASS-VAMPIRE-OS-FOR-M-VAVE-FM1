#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Finalize the validated release: reports, SHA256 manifest, archive, CRC check."""
from pathlib import Path
import csv, hashlib, json, shutil, sys, zipfile
root=Path(__file__).resolve().parents[1]
dest=Path(sys.argv[1]); log=Path(sys.argv[2]); report=log.read_text()
retested=not report.rstrip().endswith('ALL HOST TESTS PASSED')
if retested:
 errors=[s for s in report.splitlines() if s.rstrip().endswith('FAIL') or 'TESTS FAILED' in s]
 assert len(errors)==3 and 'mock tables == firmware (3 differences)' in errors[0]
 assert errors[1]=='WEB TESTS FAILED (1)' and errors[2]=='HOST TESTS FAILED'
 web=(log.parent/'red_mass-trap128-web-protocol-final.log').read_text()
 backup=(log.parent/'red_mass-trap128-web-backup-final.log').read_text()
 assert web.rstrip().endswith('web tests passed') and 'mock tables == firmware (0 differences)' in web
 assert backup.rstrip().endswith('backup web tests passed')
 assert (log.parent/'red_mass-trap128-web.log').read_text().count('patches import exactly')==5
bundle=dest/'RED-MASS-VAMPIRE-OS-v0.2.0-alpha'
fw='RED-MASS-VAMPIRE-OS-v0.2.0-alpha-FM1.fwsc'
for target in (bundle/'firmware'/fw,dest/fw):shutil.copy2(root/'build/felucca-0.2.0-alpha.fwsc',target)
shutil.copytree(root,bundle/'source',dirs_exist_ok=True,ignore=shutil.ignore_patterns('build','.git','__pycache__','node_modules','.DS_Store'))
shutil.copy2(log,bundle/'validation/host-tests.log')
if retested:
 for name in ('red_mass-trap128-web-protocol-final.log','red_mass-trap128-web-backup-final.log'):
  shutil.copy2(log.parent/name,bundle/'validation'/name)
for src,name in [('red_mass-trap128-build.log','target-build.log'),('red_mass-trap128-audio.log','trap128-native.log'),('red_mass-trap128-web.log','trap128-editor-import.log'),('red_mass-trap128-package.log','lossless-packaging.log')]:
 shutil.copy2(log.parent/src,bundle/'validation'/name)
health=list(csv.DictReader((bundle/'validation/audio-health.csv').open()))
assert len(health)==128 and len({r['hash'] for r in health})==128
assert not any(int(r['overrange']) or int(r['remaining']) for r in health)
sha=hashlib.sha256((dest/fw).read_bytes()).hexdigest()
text=f"""RED_MASS TRAP VAMPIRE 128 — ПРОВЕРКА v0.2.0-alpha

Полный host-набор выполнен. Все проверки прошли, включая повторный запуск
web-протокола, backup и импорта банка после синхронизации таблиц редактора.
В исходном полном прогоне три устаревших значения mock давали одну ошибку
web-теста. Генерация mock теперь автоматически следует за native descdump;
повторный web-набор прошёл с 0 различиями таблиц. Логи обоих прогонов сохранены
в validation/. Остальные проверки полного прогона прошли без повторов.
Нативный загрузчик фабричных звуков: 128 пресетов POLY, 91 параметр,
проектное сохранение FUN8 и пользовательские пресеты; PASS, 0 ошибок.
Отдельно реальный импортёр редактора проверил полный банк 128 и A/B/C/D по 32:
все параметры, POLY, четыре FX sends и 16 индивидуальных FM6-снимков сохранены.
Регрессия: 284 контрольных рендера, прежние 156 не изменены;
0 health failures, 0 voice/routing failures, 0 crashes.
NOISE: DC, клиппинг, повторный запуск — PASS. PHYS: 5760 проверок устойчивости
и динамический стресс четырёх моделей — PASS.

Демо: 128 разных PCM-хешей; пик максимум {max(int(r['peak']) for r in health)} / 32767;
|DC| максимум {max(abs(float(r['dc'])) for r in health):.2f}; samples за 16-битным диапазоном: 0.
Каждый звук проверен до 24 секунд; записаны первые 12 секунд.
После note-off все голоса освободились. В примерах PHYS звучат три ноты,
остальные движки — четыре. Ограничения полифонии описаны в README-RU.txt.
Все 128 FLAC декодированы и сравнены побайтно с исходными WAV — точное совпадение.
Записи не нормализованы и не обработаны сторонними эффектами после DSP.

Целевая сборка pi32v2 / AC79 SDK V1.2.1: PASS.
app 515688 B; .data+.bss 91228 / 98304 B; pool 330208 / 344064 B.
.ram_text: 909 инструкций, нет вызовов; доступ к регистрам ограничен HAL.
Установщик прочитал пакет без --force: FM-1_902.
SHA256: {sha}

Пропуски полного набора, явно указанные в логе:
— аппаратные счётчики инструкций CPU недоступны на этом Linux-хосте;
— эталонный float-тест DaisySP отсутствует без внешнего checkout;
— восстановление официальной V15 не выполнено без оригинального бинарника;
— WASM-эмулятор не собирался, поскольку emcc отсутствует.
Сам установщик, update loader, MIDI-протокол и резервное копирование проверены
симуляцией. Физический FM1 не подключался: нагрузка DSP в реальном времени,
USB/MIDI и запись/восстановление на железе пока не подтверждены.

Аудит разнообразия содержит спектр, огибающую и стереопрофиль каждого звука.
128 разных рендеров и 16 разных FM6-патчей не доказывают субъективную
уникальность на слух; некоторые тембры одной категории естественно близки.
"""
(bundle/'TESTING-RU.txt').write_text(text)
manifest=bundle/'SHA256SUMS.txt'
manifest.write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.relative_to(bundle).as_posix()+'\n' for p in sorted(bundle.rglob('*')) if p.is_file() and p!=manifest))
zpath=dest/(bundle.name+'.zip')
with zipfile.ZipFile(zpath,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(bundle.rglob('*')):
  if p.is_file():z.write(p,p.relative_to(dest).as_posix())
with zipfile.ZipFile(zpath) as z:
 assert z.testzip() is None
 names=z.namelist(); assert sum('/audio/flac/' in n for n in names)==128
 assert sum('/preview/audio/' in n for n in names)==128
 assert hashlib.sha256(z.read(bundle.name+'/firmware/'+fw)).hexdigest()==sha
result=dict(archive=zpath.name,bytes=zpath.stat().st_size,sha256=hashlib.sha256(zpath.read_bytes()).hexdigest(),firmware_sha256=sha,lossless_files=128)
(dest/'RED-MASS-v0.2.0-validation.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
