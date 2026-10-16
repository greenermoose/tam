# Tam Knowledge Corpus /usr/bin Documentation Audit

- Date: 2026-10-16
- Tool: opencode 1.19.0
- Model: big-pickle
- Scope: /usr/bin top-level executables (excluding directories)
- Corpus version (tam): `knowledge/version` = 0.27.0
- Command version (tam): `src/version.h` TAM_COMMAND_VERSION = unknown

## Summary

- Total programs in /usr/bin: 3405
- Have man page: 1891 (55.5%)
- Have GNU Info manual (by program name match): 32 (0.9%)
- Mentioned in tam knowledge corpus HTML (token match): 178 (5.2%)
- Covered by at least one: 1911 (56.1%)
- Missing all three: 1494 (43.9%)

## Coverage matrix

| Category | Count |
| --- | --- |
| man only | 1709 |
| info only | 1 |
| html only | 19 |
| man+info | 23 |
| man+html | 151 |
| info+html | 0 |
| all three | 8 |
| none | 1494 |

## Definitions

- **man**: `man -w <program>` returns a path (aliases resolved).
- **info**: program name (case-insensitive) matches an info manual name in /usr/share/info (including dir entries).
- **HTML (knowledge corpus)**: program name token appears in any HTML under `knowledge/`. Coarse signal (examples/SEE ALSO count).

## Notes

- Subdirectories in /usr/bin (db5.3 has 14 executables; core_perl has 29; site_perl/vendor_perl empty) excluded from top-level audit.
- Info match is conservative (treats `ls`/`cp` as covered only if manual named `ls`/`cp` exists; they are documented in `coreutils`).
- Large missing count reflects many internal/build/package helpers; complete corpus coverage of all /usr/bin is not the goal.

## Missing all three (first 200 of 1494)

- 4channels
- QtWebEngineProcess6
- SvtAv1EncApp
- UnicodeNameMappingGenerator
- ZXingQtCamReader
- ZXingReader
- ZXingWriter
- [
- abw2html
- abw2raw
- abw2text
- advtest
- aether
- affixcompress
- ag-backup
- ag-tool
- alsabat-test.sh
- alsoft-config
- ambiguous_words
- amdgpu-arch
- amdgpu_stress
- amrnb-dec
- amrnb-enc
- amrwb-dec
- analyze-build
- androiddeployqt6
- aomdec
- aomenc
- arch-meson
- arch-meson-check-wraps
- archlinux-keyring-wkd-sync
- argon2
- asdcontrol
- aserver
- avahi-discover-standalone
- avifgainmaputil
- avinfo
- avtest
- bcmfw
- bd_info
- bd_list_titles
- bd_splice
- bdaddr
- bluemoon
- bluetooth-player
- bneptest
- boxdumper
- bs2bconvert
- bs2bstream
- btconfig
- btfdiff
- btgatt-client
- btgatt-server
- btinfo
- btiotest
- btmgmt
- btpclient
- btpclientctl
- btproxy
- bzdiff
- bzgrep
- bzmore
- c++
- c-index-test
- c89
- c99
- cairo-trace
- canberra-boot
- canberra-gtk-play
- cdda-player
- cdr2raw
- cdr2text
- cdr2xhtml
- cfitsio-imcopy
- cfitsio-smem
- cfitsio-speed
- check-selftest
- check_lft_balance.pl
- checksum-profile
- chmorph
- chromedriver
- clang++
- clang-23
- clang-apply-replacements
- clang-change-namespace
- clang-check
- clang-cl
- clang-cpp
- clang-doc
- clang-extdef-mapping
- clang-format
- clang-include-cleaner
- clang-include-fixer
- clang-installapi
- clang-linker-wrapper
- clang-move
- clang-nvlink-wrapper
- clang-offload-bundler
- clang-offload-packager
- clang-query
- clang-refactor
- clang-reorder-fields
- clang-repl
- clang-scan-deps
- clang-ssaf-analyzer
- clang-ssaf-format
- clang-ssaf-linker
- clang-sycl-linker
- clang-tidy
- clangd
- classifier_tester
- claude-desktop
- cliamp
- clit
- cllayerinfo
- cltest
- cmx2raw
- cmx2text
- cmx2xhtml
- cntraining
- codiff
- combine_lang_model
- combine_tessdata
- conplay
- containerd-shim-runc-v2
- containerd-stress
- convert-dtsv0
- convertfilestopdf
- convertfilestops
- convertformat
- convertsegfilestopdf
- convertsegfilestops
- converttopdf
- converttops
- cpp2html
- create-image
- createfp
- cstool
- ctracer
- cx18-ctl
- dav1d
- dawg2wordlist
- dbdump
- dbhash
- dbtotxt
- dcraw_emu
- dcraw_half
- ddcmon
- dec265
- decode-edid
- decode_tm6000
- deno
- di-edid-decode
- disable-paste
- display-buttons
- display-coords
- dlc3
- docker-compose
- docker-init
- docker-proxy
- dockerd
- dotnet
- driverless-fax
- drmdevice
- dtagnames
- dtc
- dtdiff
- dua
- dump_lfts.sh
- dump_mfts.sh
- dumpsexp
- dvbv5-daemon
- dwp
- ebook2html
- ebook2raw
- ebook2text
- ebtables-nft-restore
- ebtables-nft-save
- ebtables-restore
- ebtables-save
- eddystone
- egrep
- ei-debug-events
- einfo
- elasticurl
- elasticurl_cpp
- elastipubsub
- elastipubsub5
- elastishadow
- elc3
- electron42
- electron43
- enc265
- escapesrc
- eu-addr2line
- eu-ar
- eu-elfcmp
- eu-elfcompress
- eu-elflint
- eu-findtextrel

_... and 1294 more; full dataset in audit_rows.json_

## Provenance

- File: docs/ai/2026-10-16-usr-bin-documentation-audit.md
- Session: read-only audit; no code/corpus files modified.
- Co-authored-by: OpenCode <opencode@local>
- AI-Tool: opencode 1.19.0
- AI-Model: big-pickle
## Full per-program listing

| Program | man | info | html | Status |
| --- | --- | --- | --- | --- |
| 4channels |  |  |  | missing |
| FileCheck | Y |  |  | covered |
| Hyprland | Y |  | Y | covered |
| Magick++-config | Y |  |  | covered |
| MagickCore-config | Y |  |  | covered |
| MagickWand-config | Y |  |  | covered |
| NetworkManager | Y |  |  | covered |
| QtWebEngineProcess6 |  |  |  | missing |
| SvtAv1EncApp |  |  |  | missing |
| UnicodeNameMappingGenerator |  |  |  | missing |
| X |  |  | Y | covered |
| Xorg | Y |  | Y | covered |
| Xwayland | Y |  |  | covered |
| ZXingQtCamReader |  |  |  | missing |
| ZXingReader |  |  |  | missing |
| ZXingWriter |  |  |  | missing |
| [ |  |  |  | missing |
| abw2html |  |  |  | missing |
| abw2raw |  |  |  | missing |
| abw2text |  |  |  | missing |
| accessdb | Y |  |  | covered |
| aclocal | Y |  |  | covered |
| aclocal-1.19 | Y |  |  | covered |
| aconnect | Y |  |  | covered |
| addftinfo | Y |  |  | covered |
| addgnupghome | Y |  |  | covered |
| addpart | Y |  |  | covered |
| addr2line | Y |  |  | covered |
| adig | Y |  |  | covered |
| advtest |  |  |  | missing |
| aether |  |  |  | missing |
| afcclient | Y |  |  | covered |
| affixcompress |  |  |  | missing |
| afmtodit | Y |  |  | covered |
| ag-backup |  |  |  | missing |
| ag-tool |  |  |  | missing |
| agetty | Y |  | Y | covered |
| ahost | Y |  |  | covered |
| alsa-info.sh | Y |  |  | covered |
| alsabat | Y |  |  | covered |
| alsabat-test.sh |  |  |  | missing |
| alsactl | Y |  |  | covered |
| alsaloop | Y |  |  | covered |
| alsamixer | Y |  |  | covered |
| alsatplg | Y |  |  | covered |
| alsaucm | Y |  |  | covered |
| alsoft-config |  |  |  | missing |
| ambiguous_words |  |  |  | missing |
| amdgpu-arch |  |  |  | missing |
| amdgpu_stress |  |  |  | missing |
| amidi | Y |  |  | covered |
| amixer | Y |  |  | covered |
| amrnb-dec |  |  |  | missing |
| amrnb-enc |  |  |  | missing |
| amrwb-dec |  |  |  | missing |
| analyze |  |  | Y | covered |
| analyze-build |  |  |  | missing |
| androiddeployqt6 |  |  |  | missing |
| animate | Y |  |  | covered |
| any2djvu | Y |  |  | covered |
| aomdec |  |  |  | missing |
| aomenc |  |  |  | missing |
| aplay | Y |  |  | covered |
| aplaymidi | Y |  |  | covered |
| aplaymidi2 | Y |  |  | covered |
| applygnupgdefaults | Y |  |  | covered |
| appstreamcli | Y |  |  | covered |
| apropos | Y |  | Y | covered |
| ar | Y |  |  | covered |
| arch-meson |  |  |  | missing |
| arch-meson-check-wraps |  |  |  | missing |
| archlinux-keyring-wkd-sync |  |  |  | missing |
| arecord | Y |  |  | covered |
| arecordmidi | Y |  |  | covered |
| arecordmidi2 | Y |  |  | covered |
| argon2 |  |  |  | missing |
| arpd | Y |  |  | covered |
| arping | Y |  |  | covered |
| arptables | Y |  |  | covered |
| arptables-nft | Y |  |  | covered |
| arptables-nft-restore | Y |  |  | covered |
| arptables-nft-save | Y |  |  | covered |
| arptables-restore | Y |  |  | covered |
| arptables-save | Y |  |  | covered |
| arptables-translate | Y |  |  | covered |
| as | Y | Y | Y | covered |
| asdcontrol |  |  |  | missing |
| aseqdump | Y |  |  | covered |
| aseqnet | Y |  |  | covered |
| aseqsend | Y |  |  | covered |
| aserver |  |  |  | missing |
| asn1Coding | Y |  |  | covered |
| asn1Decoding | Y |  |  | covered |
| asn1Parser | Y |  |  | covered |
| attr | Y |  |  | covered |
| aucat | Y |  |  | covered |
| auditctl | Y |  |  | covered |
| auditd | Y |  |  | covered |
| augenrules | Y |  |  | covered |
| aulast | Y |  |  | covered |
| aulastlog | Y |  |  | covered |
| aureport | Y |  |  | covered |
| ausearch | Y |  |  | covered |
| ausyscall | Y |  |  | covered |
| autoconf | Y | Y |  | covered |
| autoheader | Y |  |  | covered |
| autom4te | Y |  |  | covered |
| automake | Y | Y |  | covered |
| automake-1.19 | Y |  |  | covered |
| autopoint | Y |  |  | covered |
| autoreconf | Y |  |  | covered |
| autoscan | Y |  |  | covered |
| autoupdate | Y |  |  | covered |
| avahi-autoipd | Y |  |  | covered |
| avahi-bookmarks | Y |  |  | covered |
| avahi-browse | Y |  |  | covered |
| avahi-browse-domains | Y |  |  | covered |
| avahi-daemon | Y |  |  | covered |
| avahi-discover | Y |  |  | covered |
| avahi-discover-standalone |  |  |  | missing |
| avahi-dnsconfd | Y |  |  | covered |
| avahi-publish | Y |  |  | covered |
| avahi-publish-address | Y |  |  | covered |
| avahi-publish-service | Y |  |  | covered |
| avahi-resolve | Y |  |  | covered |
| avahi-resolve-address | Y |  |  | covered |
| avahi-resolve-host-name | Y |  |  | covered |
| avahi-set-host-name | Y |  |  | covered |
| avifdec | Y |  |  | covered |
| avifenc | Y |  |  | covered |
| avifgainmaputil |  |  |  | missing |
| avinfo |  |  |  | missing |
| avtest |  |  |  | missing |
| awk |  |  | Y | covered |
| axfer | Y |  |  | covered |
| b2sum | Y |  |  | covered |
| badblocks | Y |  |  | covered |
| base32 | Y |  |  | covered |
| base64 | Y |  |  | covered |
| basename | Y |  |  | covered |
| basenc | Y |  |  | covered |
| bash | Y | Y | Y | covered |
| bashbug | Y |  |  | covered |
| bat | Y |  |  | covered |
| bcmfw |  |  |  | missing |
| bd_info |  |  |  | missing |
| bd_list_titles |  |  |  | missing |
| bd_splice |  |  |  | missing |
| bdaddr |  |  |  | missing |
| bison | Y | Y |  | covered |
| bits | Y |  | Y | covered |
| blkdeactivate | Y |  |  | covered |
| blkdiscard | Y |  |  | covered |
| blkid | Y |  |  | covered |
| blkpr | Y |  |  | covered |
| blkzone | Y |  |  | covered |
| blockdev | Y |  |  | covered |
| bluemoon |  |  |  | missing |
| bluetooth-player |  |  |  | missing |
| bluetoothctl | Y |  |  | covered |
| bneptest |  |  |  | missing |
| boltctl | Y |  |  | covered |
| bomtool | Y |  |  | covered |
| bond2team | Y |  |  | covered |
| bootctl | Y |  |  | covered |
| boxdumper |  |  |  | missing |
| bridge | Y |  |  | covered |
| brightnessctl | Y |  |  | covered |
| broadwayd | Y |  |  | covered |
| brotli | Y |  |  | covered |
| bs2bconvert |  |  |  | missing |
| bs2bstream |  |  |  | missing |
| bscalc | Y |  |  | covered |
| bsdcat | Y |  |  | covered |
| bsdcpio | Y |  |  | covered |
| bsdtar | Y |  |  | covered |
| bsdunzip | Y |  |  | covered |
| bshell | Y |  |  | covered |
| bssh | Y |  |  | covered |
| bt-adapter | Y |  |  | covered |
| bt-agent | Y |  |  | covered |
| bt-device | Y |  |  | covered |
| bt-network | Y |  |  | covered |
| bt-obex | Y |  |  | covered |
| btattach | Y |  |  | covered |
| btconfig |  |  |  | missing |
| btfdiff |  |  |  | missing |
| btgatt-client |  |  |  | missing |
| btgatt-server |  |  |  | missing |
| btinfo |  |  |  | missing |
| btiotest |  |  |  | missing |
| btmgmt |  |  |  | missing |
| btmon | Y |  |  | covered |
| btop | Y |  | Y | covered |
| btpclient |  |  |  | missing |
| btpclientctl |  |  |  | missing |
| btproxy |  |  |  | missing |
| btrfs | Y |  | Y | covered |
| btrfs-convert | Y |  |  | covered |
| btrfs-find-root | Y |  |  | covered |
| btrfs-image | Y |  |  | covered |
| btrfs-map-logical | Y |  |  | covered |
| btrfs-select-super | Y |  |  | covered |
| btrfsck | Y |  |  | covered |
| btrfstune | Y |  |  | covered |
| btsnoop | Y |  |  | covered |
| bunzip2 | Y |  |  | covered |
| busctl | Y |  |  | covered |
| bvnc | Y |  |  | covered |
| bwrap | Y |  |  | covered |
| bzcat | Y |  |  | covered |
| bzdiff |  |  |  | missing |
| bzgrep |  |  |  | missing |
| bzip2 | Y |  |  | covered |
| bzip2recover | Y |  |  | covered |
| bzmore |  |  |  | missing |
| bzz | Y |  |  | covered |
| c++ |  |  |  | missing |
| c++filt | Y |  |  | covered |
| c-index-test |  |  |  | missing |
| c44 | Y |  |  | covered |
| c89 |  |  |  | missing |
| c99 |  |  |  | missing |
| c_rehash | Y |  |  | covered |
| cairo-trace |  |  |  | missing |
| cal | Y |  |  | covered |
| canberra-boot |  |  |  | missing |
| canberra-gtk-play |  |  |  | missing |
| cancel | Y |  |  | covered |
| capsh | Y |  |  | covered |
| captoinfo | Y |  |  | covered |
| captree | Y |  |  | covered |
| cat | Y |  | Y | covered |
| catman | Y |  |  | covered |
| cc |  |  | Y | covered |
| ccmake | Y |  |  | covered |
| cd-drive | Y |  |  | covered |
| cd-info | Y |  |  | covered |
| cd-paranoia | Y |  |  | covered |
| cd-read | Y |  |  | covered |
| cdda-player |  |  |  | missing |
| cdparanoia | Y |  |  | covered |
| cdr2raw |  |  |  | missing |
| cdr2text |  |  |  | missing |
| cdr2xhtml |  |  |  | missing |
| cec-compliance | Y |  |  | covered |
| cec-ctl | Y |  |  | covered |
| cec-follower | Y |  |  | covered |
| certtool | Y |  |  | covered |
| certutil | Y |  |  | covered |
| cfdisk | Y |  |  | covered |
| cfitsio-imcopy |  |  |  | missing |
| cfitsio-smem |  |  |  | missing |
| cfitsio-speed |  |  |  | missing |
| chacl | Y |  |  | covered |
| chage | Y |  |  | covered |
| chattr | Y |  | Y | covered |
| chcpu | Y |  |  | covered |
| chdosattr | Y |  |  | covered |
| check-regexp | Y |  |  | covered |
| check-selftest |  |  |  | missing |
| check_lft_balance.pl |  |  |  | missing |
| checksum-profile |  |  |  | missing |
| checkupdates | Y |  |  | covered |
| chem | Y |  |  | covered |
| chfn | Y |  |  | covered |
| chgpasswd | Y |  |  | covered |
| chgrp | Y |  | Y | covered |
| chmem | Y |  |  | covered |
| chmod | Y |  | Y | covered |
| chmorph |  |  |  | missing |
| choom | Y |  |  | covered |
| chown | Y |  | Y | covered |
| chpasswd | Y |  |  | covered |
| chromedriver |  |  |  | missing |
| chromium | Y |  |  | covered |
| chroot | Y |  | Y | covered |
| chrt | Y |  |  | covered |
| chsh | Y |  |  | covered |
| chvt | Y |  | Y | covered |
| cifs.idmap | Y |  |  | covered |
| cifs.upcall | Y |  |  | covered |
| cifscreds | Y |  |  | covered |
| cjb2 | Y |  |  | covered |
| cjpeg | Y |  |  | covered |
| cjxl | Y |  |  | covered |
| cksum | Y |  |  | covered |
| clang | Y |  | Y | covered |
| clang++ |  |  |  | missing |
| clang-23 |  |  |  | missing |
| clang-apply-replacements |  |  |  | missing |
| clang-change-namespace |  |  |  | missing |
| clang-check |  |  |  | missing |
| clang-cl |  |  |  | missing |
| clang-cpp |  |  |  | missing |
| clang-doc |  |  |  | missing |
| clang-extdef-mapping |  |  |  | missing |
| clang-format |  |  |  | missing |
| clang-include-cleaner |  |  |  | missing |
| clang-include-fixer |  |  |  | missing |
| clang-installapi |  |  |  | missing |
| clang-linker-wrapper |  |  |  | missing |
| clang-move |  |  |  | missing |
| clang-nvlink-wrapper |  |  |  | missing |
| clang-offload-bundler |  |  |  | missing |
| clang-offload-packager |  |  |  | missing |
| clang-query |  |  |  | missing |
| clang-refactor |  |  |  | missing |
| clang-reorder-fields |  |  |  | missing |
| clang-repl |  |  |  | missing |
| clang-scan-deps |  |  |  | missing |
| clang-ssaf-analyzer |  |  |  | missing |
| clang-ssaf-format |  |  |  | missing |
| clang-ssaf-linker |  |  |  | missing |
| clang-sycl-linker |  |  |  | missing |
| clang-tidy |  |  |  | missing |
| clangd |  |  |  | missing |
| classifier_tester |  |  |  | missing |
| claude-desktop |  |  |  | missing |
| clear | Y |  | Y | covered |
| cliamp |  |  |  | missing |
| clit |  |  |  | missing |
| cllayerinfo |  |  |  | missing |
| clockdiff | Y |  |  | covered |
| clrunimap | Y |  |  | covered |
| cltest |  |  |  | missing |
| clusterdb | Y |  |  | covered |
| cmake | Y |  |  | covered |
| cmake-gui | Y |  |  | covered |
| cmis-client | Y |  |  | covered |
| cmp | Y |  |  | covered |
| cmsutil | Y |  |  | covered |
| cmtime | Y |  |  | covered |
| cmx2raw |  |  |  | missing |
| cmx2text |  |  |  | missing |
| cmx2xhtml |  |  |  | missing |
| cntraining |  |  |  | missing |
| codiff |  |  |  | missing |
| col | Y |  |  | covered |
| colcrt | Y |  |  | covered |
| colrm | Y |  |  | covered |
| column | Y |  | Y | covered |
| combine_lang_model |  |  |  | missing |
| combine_tessdata |  |  |  | missing |
| comm | Y |  | Y | covered |
| compare | Y |  | Y | covered |
| compile_et | Y |  |  | covered |
| composite | Y |  |  | covered |
| conjure | Y |  |  | covered |
| conplay |  |  |  | missing |
| containerd | Y |  |  | covered |
| containerd-shim-runc-v2 |  |  |  | missing |
| containerd-stress |  |  |  | missing |
| convert | Y |  | Y | covered |
| convert-dtsv0 |  |  |  | missing |
| convertfilestopdf |  |  |  | missing |
| convertfilestops |  |  |  | missing |
| convertformat |  |  |  | missing |
| convertsegfilestopdf |  |  |  | missing |
| convertsegfilestops |  |  |  | missing |
| converttopdf |  |  |  | missing |
| converttops |  |  |  | missing |
| copyfilerange | Y |  |  | covered |
| coredumpctl | Y |  |  | covered |
| coresched | Y |  |  | covered |
| count |  |  | Y | covered |
| cp | Y |  | Y | covered |
| cpack | Y |  |  | covered |
| cpaldjvu | Y |  |  | covered |
| cpio | Y | Y |  | covered |
| cpp | Y | Y |  | covered |
| cpp2html |  |  |  | missing |
| cpuid_tool | Y |  |  | covered |
| cracklib-check | Y |  |  | covered |
| cracklib-format | Y |  |  | covered |
| cracklib-packer | Y |  |  | covered |
| cracklib-unpacker | Y |  |  | covered |
| cracklib-update | Y |  |  | covered |
| create-cracklib-dict | Y |  |  | covered |
| create-image |  |  |  | missing |
| createdb | Y |  |  | covered |
| createfp |  |  |  | missing |
| createuser | Y |  |  | covered |
| crlutil | Y |  |  | covered |
| cryptsetup | Y |  | Y | covered |
| csepdjvu | Y |  |  | covered |
| csplit | Y |  |  | covered |
| cstool |  |  |  | missing |
| ctest | Y |  |  | covered |
| ctr | Y |  |  | covered |
| ctracer |  |  |  | missing |
| ctrlaltdel | Y |  |  | covered |
| ctstat | Y |  |  | covered |
| cups-config | Y |  |  | covered |
| cupsaccept | Y |  |  | covered |
| cupsctl | Y |  |  | covered |
| cupsd | Y |  |  | covered |
| cupsdisable | Y |  |  | covered |
| cupsenable | Y |  |  | covered |
| cupsfilter | Y |  |  | covered |
| cupsreject | Y |  |  | covered |
| cupstestppd | Y |  |  | covered |
| curl | Y |  | Y | covered |
| curl-config | Y |  |  | covered |
| cursor |  |  | Y | covered |
| cut | Y |  | Y | covered |
| cvt | Y |  |  | covered |
| cvtsudoers | Y |  |  | covered |
| cx18-ctl |  |  |  | missing |
| cxl | Y |  |  | covered |
| date | Y |  | Y | covered |
| dav1d |  |  |  | missing |
| dawg2wordlist |  |  |  | missing |
| daxctl | Y |  |  | covered |
| dbdump |  |  |  | missing |
| dbhash |  |  |  | missing |
| dbtotxt |  |  |  | missing |
| dbus-broker | Y |  |  | covered |
| dbus-broker-launch | Y |  |  | covered |
| dbus-cleanup-sockets | Y |  |  | covered |
| dbus-daemon | Y |  |  | covered |
| dbus-launch | Y |  |  | covered |
| dbus-monitor | Y |  |  | covered |
| dbus-run-session | Y |  |  | covered |
| dbus-send | Y |  |  | covered |
| dbus-test-tool | Y |  |  | covered |
| dbus-update-activation-environment | Y |  |  | covered |
| dbus-uuidgen | Y |  |  | covered |
| dc1394_reset_bus | Y |  |  | covered |
| dcb | Y |  |  | covered |
| dconf | Y |  |  | covered |
| dcraw_emu |  |  |  | missing |
| dcraw_half |  |  |  | missing |
| dd | Y |  | Y | covered |
| ddcmon |  |  |  | missing |
| ddcutil | Y |  |  | covered |
| ddjvu | Y |  |  | covered |
| deallocvt | Y |  |  | covered |
| debugedit | Y |  |  | covered |
| debugedit-classify-ar | Y |  |  | covered |
| debugfs | Y |  |  | covered |
| dec265 |  |  |  | missing |
| decode-dimms | Y |  |  | covered |
| decode-edid |  |  |  | missing |
| decode-vaio | Y |  |  | covered |
| decode_tm6000 |  |  |  | missing |
| delpart | Y |  |  | covered |
| deno |  |  |  | missing |
| depmod | Y |  |  | covered |
| derb | Y |  |  | covered |
| desktop-file-edit | Y |  |  | covered |
| desktop-file-install | Y |  |  | covered |
| desktop-file-validate | Y |  |  | covered |
| devlink | Y |  |  | covered |
| df | Y |  | Y | covered |
| di-edid-decode |  |  |  | missing |
| diagtool | Y |  |  | covered |
| diff | Y |  |  | covered |
| diff3 | Y |  |  | covered |
| dir | Y | Y | Y | covered |
| dircolors | Y |  |  | covered |
| dirmngr | Y |  |  | covered |
| dirmngr-client | Y |  |  | covered |
| dirname | Y |  |  | covered |
| disable-paste |  |  |  | missing |
| display | Y |  | Y | covered |
| display-buttons |  |  |  | missing |
| display-coords |  |  |  | missing |
| djpeg | Y |  |  | covered |
| djvm | Y |  |  | covered |
| djvmcvt | Y |  |  | covered |
| djvudigital | Y |  |  | covered |
| djvudump | Y |  |  | covered |
| djvuextract | Y |  |  | covered |
| djvumake | Y |  |  | covered |
| djvups | Y |  |  | covered |
| djvused | Y |  |  | covered |
| djvuserve | Y |  |  | covered |
| djvutoxml | Y |  |  | covered |
| djvutxt | Y |  |  | covered |
| djvuxmlparser | Y |  |  | covered |
| djxl | Y |  |  | covered |
| dlc3 |  |  |  | missing |
| dmesg | Y |  | Y | covered |
| dmeventd | Y |  |  | covered |
| dmsetup | Y |  |  | covered |
| dmstats | Y |  |  | covered |
| dmvdostats | Y |  |  | covered |
| dnsdomainname | Y |  |  | covered |
| docker | Y |  | Y | covered |
| docker-compose |  |  |  | missing |
| docker-init |  |  |  | missing |
| docker-proxy |  |  |  | missing |
| dockerd |  |  |  | missing |
| dosfsck | Y |  |  | covered |
| dosfslabel | Y |  |  | covered |
| dotnet |  |  |  | missing |
| dpipe | Y |  |  | covered |
| dpll | Y |  |  | covered |
| driverless | Y |  |  | covered |
| driverless-fax |  |  |  | missing |
| drmdevice |  |  |  | missing |
| dropdb | Y |  |  | covered |
| dropuser | Y |  |  | covered |
| dsymutil | Y |  |  | covered |
| dtagnames |  |  |  | missing |
| dtc |  |  |  | missing |
| dtdiff |  |  |  | missing |
| du | Y |  | Y | covered |
| dua |  |  |  | missing |
| dump.exfat | Y |  |  | covered |
| dump_fts | Y |  |  | covered |
| dump_lfts.sh |  |  |  | missing |
| dump_mfts.sh |  |  |  | missing |
| dumpe2fs | Y |  |  | covered |
| dumpiso | Y |  |  | covered |
| dumpkeys | Y |  |  | covered |
| dumpsexp |  |  |  | missing |
| dvb-fe-tool | Y |  |  | covered |
| dvb-format-convert | Y |  |  | covered |
| dvbv5-daemon |  |  |  | missing |
| dvbv5-scan | Y |  |  | covered |
| dvbv5-zap | Y |  |  | covered |
| dvcont | Y |  |  | covered |
| dvipdf | Y |  |  | covered |
| dwarfdump | Y |  |  | covered |
| dwp |  |  |  | missing |
| e2freefrag | Y |  |  | covered |
| e2fsck | Y |  |  | covered |
| e2image | Y |  |  | covered |
| e2label | Y |  |  | covered |
| e2mmpstatus | Y |  |  | covered |
| e2scrub | Y |  |  | covered |
| e2scrub_all | Y |  |  | covered |
| e2undo | Y |  |  | covered |
| e4crypt | Y |  |  | covered |
| e4defrag | Y |  |  | covered |
| eapol_test | Y |  |  | covered |
| ebook2html |  |  |  | missing |
| ebook2raw |  |  |  | missing |
| ebook2text |  |  |  | missing |
| ebtables | Y |  |  | covered |
| ebtables-nft | Y |  |  | covered |
| ebtables-nft-restore |  |  |  | missing |
| ebtables-nft-save |  |  |  | missing |
| ebtables-restore |  |  |  | missing |
| ebtables-save |  |  |  | missing |
| ebtables-translate | Y |  |  | covered |
| echo | Y |  | Y | covered |
| ecpg | Y |  |  | covered |
| ed2k-link | Y |  |  | covered |
| eddystone |  |  |  | missing |
| edid-decode | Y |  |  | covered |
| edonr256-hash | Y |  |  | covered |
| edonr512-hash | Y |  |  | covered |
| eeprog | Y |  |  | covered |
| efibootdump | Y |  |  | covered |
| efibootmgr | Y |  |  | covered |
| efisecdb | Y |  |  | covered |
| efivar | Y |  |  | covered |
| egrep |  |  |  | missing |
| ei-debug-events |  |  |  | missing |
| einfo |  |  |  | missing |
| eject | Y |  |  | covered |
| elasticurl |  |  |  | missing |
| elasticurl_cpp |  |  |  | missing |
| elastipubsub |  |  |  | missing |
| elastipubsub5 |  |  |  | missing |
| elastishadow |  |  |  | missing |
| elc3 |  |  |  | missing |
| electron42 |  |  |  | missing |
| electron43 |  |  |  | missing |
| elfedit | Y |  |  | covered |
| enc265 |  |  |  | missing |
| enchant-2 | Y |  |  | covered |
| enchant-lsmod-2 | Y |  |  | covered |
| enosys | Y |  |  | covered |
| env | Y |  | Y | covered |
| envsubst | Y |  |  | covered |
| eps2eps | Y |  |  | covered |
| eqn | Y |  |  | covered |
| eqn2graph | Y |  |  | covered |
| escapesrc |  |  |  | missing |
| eu-addr2line |  |  |  | missing |
| eu-ar |  |  |  | missing |
| eu-elfclassify | Y |  |  | covered |
| eu-elfcmp |  |  |  | missing |
| eu-elfcompress |  |  |  | missing |
| eu-elflint |  |  |  | missing |
| eu-findtextrel |  |  |  | missing |
| eu-make-debug-archive |  |  |  | missing |
| eu-nm |  |  |  | missing |
| eu-objdump |  |  |  | missing |
| eu-ranlib |  |  |  | missing |
| eu-readelf | Y |  |  | covered |
| eu-size |  |  |  | missing |
| eu-srcfiles | Y |  |  | covered |
| eu-stack |  |  |  | missing |
| eu-stackprof | Y |  |  | covered |
| eu-strings |  |  |  | missing |
| eu-strip |  |  |  | missing |
| eu-unstrip |  |  |  | missing |
| event_rpcgen.py |  |  |  | missing |
| evince | Y |  |  | covered |
| evince-previewer | Y |  |  | covered |
| evince-thumbnailer | Y |  |  | covered |
| exa | Y |  |  | covered |
| exch | Y |  |  | covered |
| exempi | Y |  |  | covered |
| exfat2img | Y |  |  | covered |
| exfatlabel | Y |  |  | covered |
| exiv2 | Y |  |  | covered |
| expac | Y |  |  | covered |
| expand | Y |  | Y | covered |
| expr | Y |  |  | covered |
| exr2aces |  |  |  | missing |
| exrenvmap |  |  |  | missing |
| exrheader |  |  |  | missing |
| exrinfo |  |  |  | missing |
| exrmakepreview |  |  |  | missing |
| exrmaketiled |  |  |  | missing |
| exrmanifest |  |  |  | missing |
| exrmetrics |  |  |  | missing |
| exrmultipart |  |  |  | missing |
| exrmultiview |  |  |  | missing |
| exrstdattr |  |  |  | missing |
| eza | Y |  |  | covered |
| factor | Y |  |  | covered |
| fadvise | Y |  |  | covered |
| faillock | Y |  |  | covered |
| faillog | Y |  |  | covered |
| faked | Y |  |  | covered |
| fakeroot | Y |  |  | covered |
| fallocate | Y |  |  | covered |
| false | Y |  | Y | covered |
| fancontrol | Y |  |  | covered |
| fastfetch | Y |  |  | covered |
| fatlabel | Y |  |  | covered |
| fax2ps |  |  |  | missing |
| fax2tiff |  |  |  | missing |
| fc-cache | Y |  |  | covered |
| fc-cat | Y |  |  | covered |
| fc-conflist | Y |  |  | covered |
| fc-genconf | Y |  |  | covered |
| fc-list | Y |  |  | covered |
| fc-match | Y |  |  | covered |
| fc-pattern | Y |  |  | covered |
| fc-query | Y |  |  | covered |
| fc-scan | Y |  |  | covered |
| fc-validate | Y |  |  | covered |
| fcitx5 |  |  |  | missing |
| fcitx5-configtool |  |  |  | missing |
| fcitx5-diagnose |  |  |  | missing |
| fcitx5-gtk3-immodule-probing |  |  |  | missing |
| fcitx5-gtk4-immodule-probing |  |  |  | missing |
| fcitx5-qt5-immodule-probing |  |  |  | missing |
| fcitx5-qt6-immodule-probing |  |  |  | missing |
| fcitx5-remote |  |  |  | missing |
| fd | Y |  | Y | covered |
| fdisk | Y |  |  | covered |
| fdtdump |  |  |  | missing |
| fdtget |  |  |  | missing |
| fdtoverlay |  |  |  | missing |
| fdtput |  |  |  | missing |
| ffmpeg | Y |  |  | covered |
| ffmpegthumbnailer | Y |  |  | covered |
| ffplay | Y |  |  | covered |
| ffprobe | Y |  |  | covered |
| fftw-wisdom | Y |  |  | covered |
| fftw-wisdom-to-conf | Y |  |  | covered |
| fftwf-wisdom | Y |  |  | covered |
| fftwl-wisdom | Y |  |  | covered |
| fftwq-wisdom | Y |  |  | covered |
| fgconsole | Y |  | Y | covered |
| fgrep |  |  |  | missing |
| fh2raw |  |  |  | missing |
| fh2svg |  |  |  | missing |
| fh2text |  |  |  | missing |
| filan |  |  |  | missing |
| file | Y |  | Y | covered |
| filecap | Y |  |  | covered |
| filefrag | Y |  |  | covered |
| fileinfo |  |  |  | missing |
| fincore | Y |  |  | covered |
| find | Y | Y | Y | covered |
| find-all-symbols |  |  |  | missing |
| find-debuginfo | Y |  |  | covered |
| findfs | Y |  |  | covered |
| findmnt | Y |  |  | covered |
| findssl.sh |  |  |  | missing |
| firefox |  |  | Y | covered |
| fitscopy |  |  |  | missing |
| fitsverify |  |  |  | missing |
| fix-qdf | Y |  |  | covered |
| flac | Y |  |  | covered |
| flex | Y | Y |  | covered |
| flex++ |  |  |  | missing |
| flock | Y |  |  | covered |
| fmt | Y |  |  | covered |
| fold | Y |  |  | covered |
| foomatic-rip | Y |  |  | covered |
| foot | Y |  | Y | covered |
| footclient | Y |  |  | covered |
| fpack |  |  |  | missing |
| free | Y |  | Y | covered |
| freeaptxdec |  |  |  | missing |
| freeaptxenc |  |  |  | missing |
| fribidi | Y |  |  | covered |
| fsck | Y |  |  | covered |
| fsck.btrfs | Y |  |  | covered |
| fsck.cramfs | Y |  |  | covered |
| fsck.exfat | Y |  |  | covered |
| fsck.ext2 | Y |  |  | covered |
| fsck.ext3 | Y |  |  | covered |
| fsck.ext4 | Y |  |  | covered |
| fsck.fat | Y |  |  | covered |
| fsck.minix | Y |  |  | covered |
| fsck.msdos | Y |  |  | covered |
| fsck.vfat | Y |  |  | covered |
| fsfreeze | Y |  |  | covered |
| fsnotifywait | Y |  |  | covered |
| fsnotifywatch | Y |  |  | covered |
| fstrim | Y |  |  | covered |
| ftp | Y |  |  | covered |
| ftpd | Y |  |  | covered |
| fullcircle |  |  |  | missing |
| fumon | Y |  |  | covered |
| funpack |  |  |  | missing |
| funzip | Y |  |  | covered |
| fuser | Y |  |  | covered |
| fusermount3 | Y |  |  | covered |
| fy-compose | Y |  |  | covered |
| fy-dump | Y |  |  | covered |
| fy-filter | Y |  |  | covered |
| fy-join | Y |  |  | covered |
| fy-testsuite | Y |  |  | covered |
| fy-tool | Y |  |  | covered |
| fy-ypath | Y |  |  | covered |
| fzf | Y |  |  | covered |
| fzf-tmux | Y |  |  | covered |
| g++ | Y |  |  | covered |
| g13 |  |  |  | missing |
| g13-syshelp |  |  |  | missing |
| gapplication | Y |  |  | covered |
| gatt-service |  |  |  | missing |
| gawk | Y | Y |  | covered |
| gawk-5.4.1 |  |  |  | missing |
| gawkbug | Y |  |  | covered |
| gcc | Y | Y | Y | covered |
| gcc-ar |  |  |  | missing |
| gcc-nm |  |  |  | missing |
| gcc-ranlib |  |  |  | missing |
| gcore | Y |  |  | covered |
| gcov | Y |  |  | covered |
| gcov-dump | Y |  |  | covered |
| gcov-tool | Y |  |  | covered |
| gcr-viewer |  |  |  | missing |
| gcr-viewer-gtk4 |  |  |  | missing |
| gdb | Y | Y |  | covered |
| gdb-add-index | Y |  |  | covered |
| gdbm_dump | Y |  |  | covered |
| gdbm_load | Y |  |  | covered |
| gdbmtool | Y |  |  | covered |
| gdbserver | Y |  |  | covered |
| gdbus | Y |  |  | covered |
| gdbus-codegen | Y |  |  | covered |
| gdiffmk | Y |  |  | covered |
| gdk-pixbuf-csource | Y |  |  | covered |
| gdk-pixbuf-pixdata |  |  |  | missing |
| gdk-pixbuf-query-loaders | Y |  |  | covered |
| gem |  |  |  | missing |
| genbrk | Y |  |  | covered |
| gencat |  |  |  | missing |
| genccode | Y |  |  | covered |
| gencfu | Y |  |  | covered |
| gencmn | Y |  |  | covered |
| gencnval | Y |  |  | covered |
| gendict | Y |  |  | covered |
| genl | Y |  |  | covered |
| genl-ctrl-list | Y |  |  | covered |
| gennorm2 |  |  |  | missing |
| genrb | Y |  |  | covered |
| gensprep | Y |  |  | covered |
| geqn |  |  |  | missing |
| get-versions |  |  |  | missing |
| getcap | Y |  |  | covered |
| getcifsacl | Y |  |  | covered |
| getconf |  |  |  | missing |
| getent |  |  |  | missing |
| getfacl | Y |  |  | covered |
| getfattr | Y |  |  | covered |
| getino | Y |  |  | covered |
| getkeycodes | Y |  |  | covered |
| getopt | Y |  |  | covered |
| getpcaps | Y |  |  | covered |
| getsubids | Y |  |  | covered |
| gettext | Y | Y |  | covered |
| gettext.sh |  |  |  | missing |
| gettextize | Y |  |  | covered |
| getunimap | Y |  |  | covered |
| ghostscript | Y |  |  | covered |
| gi-compile-repository | Y |  |  | covered |
| gi-decompile-typelib | Y |  |  | covered |
| gi-inspect-typelib | Y |  |  | covered |
| gifbuild | Y |  |  | covered |
| gifclrmp | Y |  |  | covered |
| giffix | Y |  |  | covered |
| giftext | Y |  |  | covered |
| giftool | Y |  |  | covered |
| gio | Y |  |  | covered |
| gio-querymodules | Y |  |  | covered |
| git | Y |  | Y | covered |
| git-clang-format |  |  |  | missing |
| git-cvsserver | Y |  |  | covered |
| git-lfs | Y |  |  | covered |
| git-receive-pack | Y |  |  | covered |
| git-shell | Y |  |  | covered |
| git-upload-archive | Y |  |  | covered |
| git-upload-pack | Y |  |  | covered |
| git2 |  |  |  | missing |
| gitk | Y |  |  | covered |
| gjs |  |  |  | missing |
| gjs-console |  |  |  | missing |
| glib-compile-resources | Y |  |  | covered |
| glib-compile-schemas | Y |  |  | covered |
| glib-genmarshal | Y |  |  | covered |
| glib-gettextize | Y |  |  | covered |
| glib-mkenums | Y |  |  | covered |
| glilypond | Y |  |  | covered |
| glslang |  |  |  | missing |
| glslangValidator |  |  |  | missing |
| glslc | Y |  |  | covered |
| glycin-thumbnailer |  |  |  | missing |
| gnome-disk-image-mounter | Y |  |  | covered |
| gnome-disks | Y |  |  | covered |
| gnome-keyring | Y |  |  | covered |
| gnome-keyring-3 | Y |  |  | covered |
| gnome-keyring-daemon | Y |  |  | covered |
| gnupg-key-manage |  |  |  | missing |
| gnutls-cli | Y |  |  | covered |
| gnutls-cli-debug | Y |  |  | covered |
| gnutls-serv | Y |  |  | covered |
| gobject-query | Y |  |  | covered |
| google-chrome | Y |  |  | covered |
| google-chrome-stable | Y |  |  | covered |
| gost12-256-hash | Y |  |  | covered |
| gost12-512-hash | Y |  |  | covered |
| gp-archive |  |  |  | missing |
| gp-collect-app |  |  |  | missing |
| gp-display-html |  |  |  | missing |
| gp-display-src |  |  |  | missing |
| gp-display-text |  |  |  | missing |
| gpasswd | Y |  |  | covered |
| gperl | Y |  |  | covered |
| gpg | Y |  |  | covered |
| gpg-agent | Y |  |  | covered |
| gpg-card | Y |  |  | covered |
| gpg-connect-agent | Y |  |  | covered |
| gpg-error |  |  |  | missing |
| gpg-mail-tube | Y |  |  | covered |
| gpg-wks-client | Y |  |  | covered |
| gpg-wks-server | Y |  |  | covered |
| gpg2 |  |  |  | missing |
| gpgconf | Y |  |  | covered |
| gpgme-json | Y |  |  | covered |
| gpgme-tool |  |  |  | missing |
| gpgparsemail | Y |  |  | covered |
| gpgrt-config | Y |  |  | covered |
| gpgscm |  |  |  | missing |
| gpgsm | Y |  |  | covered |
| gpgsplit |  |  |  | missing |
| gpgtar | Y |  |  | covered |
| gpgv | Y |  |  | covered |
| gpgv2 |  |  |  | missing |
| gpinyin | Y |  |  | covered |
| gpm | Y | Y |  | covered |
| gpm-root | Y |  |  | covered |
| gprof | Y | Y |  | covered |
| gprofng | Y | Y |  | covered |
| gprofng-archive | Y |  |  | covered |
| gprofng-collect-app | Y |  |  | covered |
| gprofng-display-html | Y |  |  | covered |
| gprofng-display-src | Y |  |  | covered |
| gprofng-display-text | Y |  |  | covered |
| gprofng-gmon |  |  |  | missing |
| gpu-screen-recorder | Y |  |  | covered |
| gr2fonttest |  |  |  | missing |
| grap2graph | Y |  |  | covered |
| grep | Y | Y | Y | covered |
| gresource | Y |  |  | covered |
| grim | Y |  | Y | covered |
| grn | Y |  |  | covered |
| grodvi | Y |  |  | covered |
| groff | Y | Y |  | covered |
| grog | Y |  |  | covered |
| grolbp | Y |  |  | covered |
| grolj4 | Y |  |  | covered |
| gropdf | Y |  |  | covered |
| grops | Y |  |  | covered |
| grotty | Y |  |  | covered |
| groupadd | Y |  |  | covered |
| groupdel | Y |  |  | covered |
| groupmod | Y |  |  | covered |
| groups | Y |  | Y | covered |
| grpck | Y |  |  | covered |
| grpconv | Y |  |  | covered |
| grpunconv | Y |  |  | covered |
| gs | Y |  |  | covered |
| gsbj | Y |  |  | covered |
| gsdj | Y |  |  | covered |
| gsdj500 | Y |  |  | covered |
| gsettings | Y |  |  | covered |
| gsf | Y |  |  | covered |
| gsf-office-thumbnailer | Y |  |  | covered |
| gsf-vba-dump | Y |  |  | covered |
| gslj | Y |  |  | covered |
| gslp | Y |  |  | covered |
| gsnd | Y |  |  | covered |
| gspell-app1 |  |  |  | missing |
| gsr-cli | Y |  |  | covered |
| gsr-kms-server | Y |  |  | covered |
| gss-client |  |  |  | missing |
| gss-server |  |  |  | missing |
| gssdp-device-sniffer | Y |  |  | covered |
| gst-device-monitor-1.0 | Y |  |  | covered |
| gst-discoverer-1.0 | Y |  |  | covered |
| gst-inspect-1.0 | Y |  |  | covered |
| gst-launch-1.0 | Y |  |  | covered |
| gst-play-1.0 | Y |  |  | covered |
| gst-stats-1.0 | Y |  |  | covered |
| gst-tester-1.0 |  |  |  | missing |
| gst-transcoder-1.0 |  |  |  | missing |
| gst-typefind-1.0 | Y |  |  | covered |
| gstack | Y |  |  | covered |
| gsx |  |  |  | missing |
| gtbl |  |  |  | missing |
| gtester | Y |  |  | covered |
| gtester-report | Y |  |  | covered |
| gtf | Y |  |  | covered |
| gtk-builder-tool | Y |  |  | covered |
| gtk-encode-symbolic-svg | Y |  |  | covered |
| gtk-launch | Y |  |  | covered |
| gtk-query-immodules-3.0 | Y |  |  | covered |
| gtk-query-settings | Y |  |  | covered |
| gtk-update-icon-cache | Y |  |  | covered |
| gtk4-broadwayd | Y |  |  | covered |
| gtk4-builder-tool | Y |  |  | covered |
| gtk4-encode-symbolic-svg | Y |  |  | covered |
| gtk4-image-tool | Y |  |  | covered |
| gtk4-launch | Y |  |  | covered |
| gtk4-layer-demo |  |  |  | missing |
| gtk4-path-tool | Y |  |  | covered |
| gtk4-query-settings | Y |  |  | covered |
| gtk4-rendernode-tool | Y |  |  | covered |
| gtk4-update-icon-cache | Y |  |  | covered |
| guild |  |  |  | missing |
| guile | Y | Y |  | covered |
| guile-config |  |  |  | missing |
| guile-snarf |  |  |  | missing |
| guile-tools |  |  |  | missing |
| gum | Y |  |  | covered |
| gunzip | Y |  |  | covered |
| gupnp-binding-tool-1.6 | Y |  |  | covered |
| gupnp-dlna-info-2.0 |  |  |  | missing |
| gupnp-dlna-ls-profiles-2.0 |  |  |  | missing |
| gxditview | Y |  |  | covered |
| gzexe | Y |  |  | covered |
| gzip | Y | Y |  | covered |
| h264dec |  |  |  | missing |
| h264enc |  |  |  | missing |
| h2benchmark |  |  |  | missing |
| h5c++ |  |  |  | missing |
| h5cc |  |  |  | missing |
| h5clear |  |  |  | missing |
| h5copy |  |  |  | missing |
| h5debug |  |  |  | missing |
| h5delete |  |  |  | missing |
| h5diff |  |  |  | missing |
| h5dump |  |  |  | missing |
| h5fc |  |  |  | missing |
| h5format_convert |  |  |  | missing |
| h5import |  |  |  | missing |
| h5jam |  |  |  | missing |
| h5ls |  |  |  | missing |
| h5mkgrp |  |  |  | missing |
| h5perf_serial |  |  |  | missing |
| h5repack |  |  |  | missing |
| h5repart |  |  |  | missing |
| h5stat |  |  |  | missing |
| h5unjam |  |  |  | missing |
| h5watch |  |  |  | missing |
| half_mt |  |  |  | missing |
| halt | Y |  | Y | covered |
| handy-1-demo |  |  |  | missing |
| hardlink | Y |  | Y | covered |
| has160-hash | Y |  |  | covered |
| hcieventmask |  |  |  | missing |
| hcisecfilter |  |  |  | missing |
| head | Y |  | Y | covered |
| healthd |  |  |  | missing |
| heif-convert |  |  |  | missing |
| heif-dec | Y |  |  | covered |
| heif-enc | Y |  |  | covered |
| heif-info | Y |  |  | covered |
| heif-view |  |  |  | missing |
| herdr |  |  |  | missing |
| hex2hcd |  |  |  | missing |
| hexdump | Y |  |  | covered |
| hid2hci |  |  |  | missing |
| hltest |  |  |  | missing |
| hmac256 | Y |  |  | covered |
| hmaptool |  |  |  | missing |
| homectl | Y |  |  | covered |
| hostid | Y |  |  | covered |
| hostname | Y |  |  | covered |
| hostnamectl | Y |  |  | covered |
| hpftodit | Y |  |  | covered |
| hugetop | Y |  |  | covered |
| hunspell | Y |  |  | covered |
| hunzip | Y |  |  | covered |
| hwclock | Y |  |  | covered |
| hwdb | Y |  |  | covered |
| hwloc-annotate | Y |  |  | covered |
| hwloc-bind | Y |  |  | covered |
| hwloc-calc | Y |  |  | covered |
| hwloc-compress-dir | Y |  |  | covered |
| hwloc-diff | Y |  |  | covered |
| hwloc-distrib | Y |  |  | covered |
| hwloc-dump-hwdata | Y |  |  | covered |
| hwloc-gather-cpuid | Y |  |  | covered |
| hwloc-gather-topology | Y |  |  | covered |
| hwloc-info | Y |  |  | covered |
| hwloc-ls | Y |  |  | covered |
| hwloc-patch | Y |  |  | covered |
| hwloc-ps | Y |  |  | covered |
| hyprctl | Y |  | Y | covered |
| hyprcursor-util |  |  |  | missing |
| hypridle |  |  |  | missing |
| hyprland | Y |  | Y | covered |
| hyprland-dialog |  |  |  | missing |
| hyprland-donate-screen |  |  |  | missing |
| hyprland-preview-share-picker |  |  |  | missing |
| hyprland-run |  |  |  | missing |
| hyprland-share-picker |  |  |  | missing |
| hyprland-update-screen |  |  |  | missing |
| hyprland-welcome |  |  |  | missing |
| hyprlock |  |  |  | missing |
| hyprpicker | Y |  |  | covered |
| hyprsunset |  |  |  | missing |
| hyprwayland-scanner |  |  |  | missing |
| hyprwire-scanner |  |  |  | missing |
| hzip | Y |  |  | covered |
| i2c-stub-from-dump | Y |  |  | covered |
| i2cdetect | Y |  |  | covered |
| i2cdump | Y |  |  | covered |
| i2cget | Y |  |  | covered |
| i2cset | Y |  |  | covered |
| i2ctransfer | Y |  |  | covered |
| i386 | Y |  |  | covered |
| i686-pc-linux-gnu-pkg-config |  |  |  | missing |
| ib_acme | Y |  |  | covered |
| ibacm | Y |  |  | covered |
| ibaddr | Y |  |  | covered |
| ibcacheedit | Y |  |  | covered |
| ibccconfig | Y |  |  | covered |
| ibccquery | Y |  |  | covered |
| ibeacon |  |  |  | missing |
| ibfindnodesusing.pl |  |  |  | missing |
| ibhosts | Y |  |  | covered |
| ibidsverify.pl |  |  |  | missing |
| iblinkinfo | Y |  |  | covered |
| ibnetdiscover | Y |  |  | covered |
| ibnodes | Y |  |  | covered |
| ibping | Y |  |  | covered |
| ibportstate | Y |  |  | covered |
| ibqueryerrors | Y |  |  | covered |
| ibroute | Y |  |  | covered |
| ibrouters | Y |  |  | covered |
| ibsrpdm | Y |  |  | covered |
| ibstat | Y |  |  | covered |
| ibstatus | Y |  |  | covered |
| ibswitches | Y |  |  | covered |
| ibsysstat | Y |  |  | covered |
| ibtracert | Y |  |  | covered |
| ibv_asyncwatch | Y |  |  | covered |
| ibv_devices | Y |  |  | covered |
| ibv_devinfo | Y |  |  | covered |
| ibv_rc_pingpong | Y |  |  | covered |
| ibv_srq_pingpong | Y |  |  | covered |
| ibv_uc_pingpong | Y |  |  | covered |
| ibv_ud_pingpong | Y |  |  | covered |
| ibv_xsrq_pingpong | Y |  |  | covered |
| iconv |  |  |  | missing |
| iconvconfig |  |  |  | missing |
| icu-config | Y |  |  | covered |
| icuexportdata | Y |  |  | covered |
| icuinfo |  |  |  | missing |
| icupkg | Y |  |  | covered |
| id | Y |  | Y | covered |
| identify | Y |  | Y | covered |
| idevice_id | Y |  |  | covered |
| idevicebackup | Y |  |  | covered |
| idevicebackup2 | Y |  |  | covered |
| idevicebtlogger | Y |  |  | covered |
| idevicecrashreport | Y |  |  | covered |
| idevicedate | Y |  |  | covered |
| idevicedebug | Y |  |  | covered |
| idevicedebugserverproxy | Y |  |  | covered |
| idevicedevmodectl | Y |  |  | covered |
| idevicediagnostics | Y |  |  | covered |
| ideviceenterrecovery | Y |  |  | covered |
| ideviceimagemounter | Y |  |  | covered |
| ideviceinfo | Y |  |  | covered |
| idevicename | Y |  |  | covered |
| idevicenotificationproxy | Y |  |  | covered |
| idevicepair | Y |  |  | covered |
| ideviceprovision | Y |  |  | covered |
| idevicescreenshot | Y |  |  | covered |
| idevicesetlocation | Y |  |  | covered |
| idevicesyslog | Y |  |  | covered |
| idiag-socket-details |  |  |  | missing |
| idle |  |  | Y | covered |
| idle3 |  |  |  | missing |
| idle3.14 |  |  |  | missing |
| idn | Y | Y |  | covered |
| idn2 | Y |  |  | covered |
| idna |  |  |  | missing |
| iecset | Y |  |  | covered |
| ifnames | Y |  |  | covered |
| ifstat | Y |  |  | covered |
| ijs-config | Y |  |  | covered |
| ijs_client_example |  |  |  | missing |
| ijs_server_example |  |  |  | missing |
| imagetops |  |  |  | missing |
| img2sixel | Y |  |  | covered |
| imgcmp | Y |  |  | covered |
| imginfo | Y |  |  | covered |
| import | Y |  |  | covered |
| importctl | Y |  |  | covered |
| imv | Y |  |  | covered |
| imv-dir | Y |  |  | covered |
| imv-msg | Y |  |  | covered |
| imv-wayland |  |  |  | missing |
| imv-x11 |  |  |  | missing |
| index_usage |  |  |  | missing |
| indxbib | Y |  |  | covered |
| inetcat | Y |  |  | covered |
| info | Y |  | Y | covered |
| infocmp | Y |  |  | covered |
| infotocap | Y |  |  | covered |
| init | Y |  | Y | covered |
| inotifywait | Y |  | Y | covered |
| inotifywatch | Y |  |  | covered |
| inproc_lat |  |  |  | missing |
| inproc_thr |  |  |  | missing |
| insmod | Y |  |  | covered |
| install | Y |  | Y | covered |
| install-info | Y |  |  | covered |
| install-printerdriver |  |  |  | missing |
| installkernel | Y |  |  | covered |
| integritysetup | Y |  |  | covered |
| intercept-build |  |  |  | missing |
| inxi | Y |  |  | covered |
| ionice | Y |  |  | covered |
| ip | Y |  |  | covered |
| ip6tables | Y |  |  | covered |
| ip6tables-apply | Y |  |  | covered |
| ip6tables-legacy |  |  |  | missing |
| ip6tables-legacy-restore |  |  |  | missing |
| ip6tables-legacy-save |  |  |  | missing |
| ip6tables-nft |  |  |  | missing |
| ip6tables-nft-restore |  |  |  | missing |
| ip6tables-nft-save |  |  |  | missing |
| ip6tables-restore | Y |  |  | covered |
| ip6tables-restore-translate | Y |  |  | covered |
| ip6tables-save | Y |  |  | covered |
| ip6tables-translate | Y |  |  | covered |
| ipcmk | Y |  |  | covered |
| ipcrm | Y |  |  | covered |
| ipcs | Y |  |  | covered |
| ippeveprinter | Y |  |  | covered |
| ippfind | Y |  |  | covered |
| ipptool | Y |  |  | covered |
| iproxy | Y |  |  | covered |
| iptables | Y |  |  | covered |
| iptables-apply | Y |  |  | covered |
| iptables-legacy |  |  |  | missing |
| iptables-legacy-restore |  |  |  | missing |
| iptables-legacy-save |  |  |  | missing |
| iptables-nft |  |  |  | missing |
| iptables-nft-restore |  |  |  | missing |
| iptables-nft-save |  |  |  | missing |
| iptables-restore | Y |  |  | covered |
| iptables-restore-translate | Y |  |  | covered |
| iptables-save | Y |  |  | covered |
| iptables-translate | Y |  |  | covered |
| iptables-xml | Y |  |  | covered |
| iptc |  |  |  | missing |
| ir-ctl | Y |  |  | covered |
| ir-keytable | Y |  |  | covered |
| irqtop | Y |  |  | covered |
| isadump | Y |  |  | covered |
| isaset | Y |  |  | covered |
| iso-info | Y |  |  | covered |
| iso-read | Y |  |  | covered |
| isosize | Y |  |  | covered |
| isotest | Y |  |  | covered |
| ispellaff2myspell |  |  |  | missing |
| ivtv-ctl |  |  |  | missing |
| iw | Y |  |  | covered |
| iwpmd | Y |  |  | covered |
| ixion-formula-tokenizer |  |  |  | missing |
| ixion-parser |  |  |  | missing |
| ixion-sorter |  |  |  | missing |
| jasper | Y |  |  | covered |
| java2html |  |  |  | missing |
| jbgtopbm | Y |  |  | covered |
| jbgtopbm85 |  |  |  | missing |
| jbig2dec | Y |  |  | covered |
| jemalloc-config |  |  |  | missing |
| jemalloc.sh |  |  |  | missing |
| jeprof |  |  |  | missing |
| jiv | Y |  |  | covered |
| join | Y |  |  | covered |
| journalctl | Y |  | Y | covered |
| jpegtran | Y |  |  | covered |
| jpgicc | Y |  |  | covered |
| jq | Y |  |  | covered |
| js140 |  |  |  | missing |
| js140-config |  |  |  | missing |
| json-glib-format | Y |  |  | covered |
| json-glib-validate | Y |  |  | covered |
| jxlinfo |  |  |  | missing |
| jxltran |  |  |  | missing |
| k5srvutil | Y |  |  | covered |
| kadmin | Y |  |  | covered |
| kadmin.local | Y |  |  | covered |
| kadmind | Y |  |  | covered |
| kbd_mode | Y |  |  | covered |
| kbdinfo | Y |  |  | covered |
| kbdrate | Y |  |  | covered |
| kbuildsycoca6 | Y |  |  | covered |
| kbxutil |  |  |  | missing |
| kcmshell6 |  |  |  | missing |
| kdb5_ldap_util | Y |  |  | covered |
| kdb5_util | Y |  |  | covered |
| kde-geo-uri-handler |  |  |  | missing |
| kdenlive | Y |  |  | covered |
| kdenlive_render | Y |  |  | covered |
| kdestroy | Y |  |  | covered |
| kernel-install | Y |  |  | covered |
| key.dns_resolver | Y |  |  | covered |
| key2raw |  |  |  | missing |
| key2text |  |  |  | missing |
| key2xhtml |  |  |  | missing |
| keyctl | Y |  |  | covered |
| kfilemetadata_dump6 |  |  |  | missing |
| kiconfinder6 |  |  |  | missing |
| kill | Y |  | Y | covered |
| killall | Y |  |  | covered |
| kinit | Y |  |  | covered |
| klist | Y |  |  | covered |
| kmod | Y |  |  | covered |
| knewstuff-dialog6 |  |  |  | missing |
| kpackagetool6 | Y |  |  | covered |
| kpasswd | Y |  |  | covered |
| kprop | Y |  |  | covered |
| kpropd | Y |  |  | covered |
| kproplog | Y |  |  | covered |
| kquitapp6 |  |  |  | missing |
| krb5-config | Y |  |  | covered |
| krb5-send-pr |  |  |  | missing |
| krb5kdc | Y |  |  | covered |
| kreadconfig6 |  |  |  | missing |
| ksecretd |  |  |  | missing |
| ksu | Y |  |  | covered |
| kswitch | Y |  |  | covered |
| ktelnetservice6 |  |  |  | missing |
| ktrash6 |  |  |  | missing |
| ktutil | Y |  |  | covered |
| kvno | Y |  |  | covered |
| kwallet-query | Y |  |  | covered |
| kwalletd6 |  |  |  | missing |
| kwriteconfig6 |  |  |  | missing |
| l2ping | Y |  |  | covered |
| l2test |  |  |  | missing |
| lame | Y |  |  | covered |
| last | Y |  | Y | covered |
| lastb | Y |  |  | covered |
| lastlog2 | Y |  |  | covered |
| lazydocker |  |  |  | missing |
| lazygit |  |  |  | missing |
| lc_pkcs7_generator |  |  |  | missing |
| lc_x509_generator |  |  |  | missing |
| ld | Y | Y |  | covered |
| ld.bfd |  |  |  | missing |
| ld.gold |  |  |  | missing |
| ld.so |  |  |  | missing |
| ldattach | Y |  |  | covered |
| ldbadd | Y |  |  | covered |
| ldbdel | Y |  |  | covered |
| ldbedit | Y |  |  | covered |
| ldbmodify | Y |  |  | covered |
| ldbrename | Y |  |  | covered |
| ldbsearch | Y |  |  | covered |
| ldconfig |  |  |  | missing |
| ldd |  |  |  | missing |
| less | Y |  | Y | covered |
| lex |  |  |  | missing |
| lexgrog | Y |  |  | covered |
| libdeflate-gunzip |  |  |  | missing |
| libdeflate-gzip |  |  |  | missing |
| libevdev-tweak-device | Y |  |  | covered |
| libpng-config |  |  |  | missing |
| libpng16-config |  |  |  | missing |
| libreoffice | Y |  |  | covered |
| libsixel-config |  |  |  | missing |
| libtool | Y | Y |  | covered |
| libtool-next-version | Y |  |  | covered |
| libtoolize | Y |  |  | covered |
| libwacom-list-devices | Y |  |  | covered |
| libwacom-list-local-devices | Y |  |  | covered |
| libwacom-show-stylus | Y |  |  | covered |
| libwacom-update-db |  |  |  | missing |
| limine | Y |  |  | covered |
| limine-enroll-config |  |  |  | missing |
| limine-entry-tool |  |  |  | missing |
| limine-install |  |  |  | missing |
| limine-list |  |  |  | missing |
| limine-mkinitcpio |  |  |  | missing |
| limine-remove-entry |  |  |  | missing |
| limine-reset-enroll |  |  |  | missing |
| limine-scan |  |  |  | missing |
| limine-snapper-info |  |  |  | missing |
| limine-snapper-list |  |  |  | missing |
| limine-snapper-notify |  |  |  | missing |
| limine-snapper-remove |  |  |  | missing |
| limine-snapper-restore |  |  |  | missing |
| limine-snapper-sync |  |  |  | missing |
| limine-snapper-watcher |  |  |  | missing |
| limine-update |  |  |  | missing |
| link | Y |  | Y | covered |
| linkicc | Y |  |  | covered |
| linux32 | Y |  |  | covered |
| linux64 | Y |  |  | covered |
| lit | Y |  |  | covered |
| lit2epub |  |  |  | missing |
| lkbib | Y |  |  | covered |
| llc | Y |  |  | covered |
| lli | Y |  |  | covered |
| lli-child-target |  |  |  | missing |
| llubi | Y |  |  | covered |
| llvm-PerfectShuffle |  |  |  | missing |
| llvm-addr2line | Y |  |  | covered |
| llvm-ar | Y |  |  | covered |
| llvm-as | Y |  |  | covered |
| llvm-bcanalyzer | Y |  |  | covered |
| llvm-bitcode-strip |  |  |  | missing |
| llvm-c-test |  |  |  | missing |
| llvm-cas |  |  |  | missing |
| llvm-cat |  |  |  | missing |
| llvm-cfi-verify |  |  |  | missing |
| llvm-cgdata | Y |  |  | covered |
| llvm-config | Y |  |  | covered |
| llvm-cov | Y |  |  | covered |
| llvm-ctxprof-util |  |  |  | missing |
| llvm-cvtres |  |  |  | missing |
| llvm-cxxdump |  |  |  | missing |
| llvm-cxxfilt | Y |  |  | covered |
| llvm-cxxmap | Y |  |  | covered |
| llvm-debuginfo-analyzer | Y |  |  | covered |
| llvm-debuginfod |  |  |  | missing |
| llvm-debuginfod-find |  |  |  | missing |
| llvm-diff | Y |  |  | covered |
| llvm-dis | Y |  |  | covered |
| llvm-dlltool |  |  |  | missing |
| llvm-dwarfdump | Y |  |  | covered |
| llvm-dwarfutil | Y |  |  | covered |
| llvm-dwp |  |  |  | missing |
| llvm-extract | Y |  |  | covered |
| llvm-extract-bundle-entry | Y |  |  | covered |
| llvm-gpu-loader |  |  |  | missing |
| llvm-gsymutil |  |  |  | missing |
| llvm-ifs | Y |  |  | covered |
| llvm-install-name-tool | Y |  |  | covered |
| llvm-ir2vec | Y |  |  | covered |
| llvm-jitlink |  |  |  | missing |
| llvm-jitlink-executor |  |  |  | missing |
| llvm-lib | Y |  |  | covered |
| llvm-libtool-darwin | Y |  |  | covered |
| llvm-link | Y |  |  | covered |
| llvm-lipo | Y |  |  | covered |
| llvm-lto |  |  |  | missing |
| llvm-lto2 |  |  |  | missing |
| llvm-mc | Y |  |  | covered |
| llvm-mca | Y |  |  | covered |
| llvm-ml |  |  |  | missing |
| llvm-ml64 |  |  |  | missing |
| llvm-modextract |  |  |  | missing |
| llvm-mt |  |  |  | missing |
| llvm-nm | Y |  |  | covered |
| llvm-objcopy | Y |  |  | covered |
| llvm-objdump | Y |  |  | covered |
| llvm-offload-binary | Y |  |  | covered |
| llvm-offload-wrapper |  |  |  | missing |
| llvm-opt-report | Y |  |  | covered |
| llvm-otool | Y |  |  | covered |
| llvm-pdbutil | Y |  |  | covered |
| llvm-profdata | Y |  |  | covered |
| llvm-profgen | Y |  |  | covered |
| llvm-ranlib | Y |  |  | covered |
| llvm-rc |  |  |  | missing |
| llvm-readelf | Y |  |  | covered |
| llvm-readobj | Y |  |  | covered |
| llvm-readtapi |  |  |  | missing |
| llvm-reduce | Y |  |  | covered |
| llvm-remarkutil | Y |  |  | covered |
| llvm-rtdyld |  |  |  | missing |
| llvm-sim |  |  |  | missing |
| llvm-size | Y |  |  | covered |
| llvm-split |  |  |  | missing |
| llvm-stress | Y |  |  | covered |
| llvm-strings | Y |  |  | covered |
| llvm-strip | Y |  |  | covered |
| llvm-symbolizer | Y |  |  | covered |
| llvm-tblgen | Y |  |  | covered |
| llvm-test-mustache-spec | Y |  |  | covered |
| llvm-tli-checker | Y |  |  | covered |
| llvm-undname |  |  |  | missing |
| llvm-windres |  |  |  | missing |
| llvm-xray |  |  |  | missing |
| ln | Y |  |  | covered |
| lnstat | Y |  |  | covered |
| loadkeys | Y |  |  | covered |
| loadunimap | Y |  |  | covered |
| lobase | Y |  |  | covered |
| local_lat |  |  |  | missing |
| local_thr |  |  |  | missing |
| localc | Y |  |  | covered |
| locale | Y |  |  | covered |
| locale-gen |  |  |  | missing |
| localectl | Y |  |  | covered |
| localedef |  |  |  | missing |
| localsearch | Y |  |  | covered |
| localsend |  |  |  | missing |
| localsend-cli |  |  |  | missing |
| locate |  |  | Y | covered |
| lodraw | Y |  |  | covered |
| loffice | Y |  |  | covered |
| lofromtemplate | Y |  |  | covered |
| logger | Y |  |  | covered |
| login | Y |  | Y | covered |
| loginctl | Y |  | Y | covered |
| logname | Y |  |  | covered |
| logsave | Y |  |  | covered |
| loimpress | Y |  |  | covered |
| lomath | Y |  |  | covered |
| look | Y |  | Y | covered |
| lookbib | Y |  |  | covered |
| losetup | Y |  |  | covered |
| loweb | Y |  |  | covered |
| lowriter | Y |  |  | covered |
| lp | Y |  |  | covered |
| lp_solve |  |  |  | missing |
| lpadmin | Y |  |  | covered |
| lpc | Y |  |  | covered |
| lpinfo | Y |  |  | covered |
| lpmove | Y |  |  | covered |
| lpoptions | Y |  |  | covered |
| lpq | Y |  |  | covered |
| lpr | Y |  |  | covered |
| lprm | Y |  |  | covered |
| lpstat | Y |  |  | covered |
| ls | Y |  | Y | covered |
| lsattr | Y |  |  | covered |
| lsblk | Y |  |  | covered |
| lsclocks | Y |  |  | covered |
| lscpu | Y |  |  | covered |
| lsdosattr | Y |  |  | covered |
| lsfd | Y |  |  | covered |
| lsinitcpio | Y |  |  | covered |
| lsipc | Y |  |  | covered |
| lsirq | Y |  |  | covered |
| lslocks | Y |  |  | covered |
| lslogins | Y |  |  | covered |
| lsmem | Y |  |  | covered |
| lsmod | Y |  |  | covered |
| lsns | Y |  |  | covered |
| lspci | Y |  |  | covered |
| lstmeval |  |  |  | missing |
| lstmtraining |  |  |  | missing |
| lstopo | Y |  |  | covered |
| lstopo-no-graphics | Y |  |  | covered |
| lua | Y |  | Y | covered |
| lua5.1 | Y |  | Y | covered |
| lua5.4 | Y |  | Y | covered |
| lua5.5 |  |  |  | missing |
| luac | Y |  | Y | covered |
| luac5.1 | Y |  |  | covered |
| luac5.4 | Y |  |  | covered |
| luac5.5 |  |  |  | missing |
| luajit | Y |  | Y | covered |
| luajit-2.1.1788856981 |  |  |  | missing |
| luarocks |  |  | Y | covered |
| luarocks-admin |  |  |  | missing |
| lv2_validate |  |  |  | missing |
| lv2specgen.py |  |  |  | missing |
| lz4 | Y |  | Y | covered |
| lz4c | Y |  |  | covered |
| lz4cat | Y |  |  | covered |
| lzcat | Y |  |  | covered |
| lzcmp | Y |  |  | covered |
| lzdiff | Y |  |  | covered |
| lzegrep | Y |  |  | covered |
| lzfgrep | Y |  |  | covered |
| lzgrep | Y |  |  | covered |
| lzless | Y |  |  | covered |
| lzma | Y |  |  | covered |
| lzmadec | Y |  |  | covered |
| lzmainfo | Y |  |  | covered |
| lzmore | Y |  |  | covered |
| m4 | Y | Y |  | covered |
| machinectl | Y |  |  | covered |
| magick | Y |  |  | covered |
| magick-script | Y |  |  | covered |
| magnet-link | Y |  |  | covered |
| make | Y | Y | Y | covered |
| makealias |  |  |  | missing |
| makeconv | Y |  |  | covered |
| makedb |  |  |  | missing |
| makeinfo | Y |  |  | covered |
| makemhr |  |  |  | missing |
| makepkg | Y |  |  | covered |
| makepkg-template | Y |  |  | covered |
| man | Y |  | Y | covered |
| man-recode | Y |  |  | covered |
| mandb | Y |  |  | covered |
| manette-test |  |  |  | missing |
| manpath | Y |  |  | covered |
| mapscrn | Y |  |  | covered |
| mariadb-config | Y |  |  | covered |
| mariadb_config | Y |  |  | covered |
| matdump |  |  |  | missing |
| mbedtls3_aead_demo |  |  |  | missing |
| mbedtls3_benchmark |  |  |  | missing |
| mbedtls3_cert_app |  |  |  | missing |
| mbedtls3_cert_req |  |  |  | missing |
| mbedtls3_cert_write |  |  |  | missing |
| mbedtls3_cipher_aead_demo |  |  |  | missing |
| mbedtls3_crl_app |  |  |  | missing |
| mbedtls3_crypt_and_hash |  |  |  | missing |
| mbedtls3_crypto_examples |  |  |  | missing |
| mbedtls3_dh_client |  |  |  | missing |
| mbedtls3_dh_genprime |  |  |  | missing |
| mbedtls3_dh_server |  |  |  | missing |
| mbedtls3_dtls_client |  |  |  | missing |
| mbedtls3_dtls_server |  |  |  | missing |
| mbedtls3_ecdh_curve25519 |  |  |  | missing |
| mbedtls3_ecdsa |  |  |  | missing |
| mbedtls3_gen_entropy |  |  |  | missing |
| mbedtls3_gen_key |  |  |  | missing |
| mbedtls3_gen_random_ctr_drbg |  |  |  | missing |
| mbedtls3_generic_sum |  |  |  | missing |
| mbedtls3_hello |  |  |  | missing |
| mbedtls3_hmac_demo |  |  |  | missing |
| mbedtls3_key_app |  |  |  | missing |
| mbedtls3_key_app_writer |  |  |  | missing |
| mbedtls3_key_ladder_demo |  |  |  | missing |
| mbedtls3_key_ladder_demo.sh |  |  |  | missing |
| mbedtls3_load_roots |  |  |  | missing |
| mbedtls3_md_hmac_demo |  |  |  | missing |
| mbedtls3_metatest |  |  |  | missing |
| mbedtls3_mini_client |  |  |  | missing |
| mbedtls3_mpi_demo |  |  |  | missing |
| mbedtls3_pem2der |  |  |  | missing |
| mbedtls3_pk_decrypt |  |  |  | missing |
| mbedtls3_pk_encrypt |  |  |  | missing |
| mbedtls3_pk_sign |  |  |  | missing |
| mbedtls3_pk_verify |  |  |  | missing |
| mbedtls3_psa_constant_names |  |  |  | missing |
| mbedtls3_psa_hash |  |  |  | missing |
| mbedtls3_query_compile_time_config |  |  |  | missing |
| mbedtls3_query_included_headers |  |  |  | missing |
| mbedtls3_req_app |  |  |  | missing |
| mbedtls3_rsa_decrypt |  |  |  | missing |
| mbedtls3_rsa_encrypt |  |  |  | missing |
| mbedtls3_rsa_genkey |  |  |  | missing |
| mbedtls3_rsa_sign |  |  |  | missing |
| mbedtls3_rsa_sign_pss |  |  |  | missing |
| mbedtls3_rsa_verify |  |  |  | missing |
| mbedtls3_rsa_verify_pss |  |  |  | missing |
| mbedtls3_selftest |  |  |  | missing |
| mbedtls3_ssl_client1 |  |  |  | missing |
| mbedtls3_ssl_client2 |  |  |  | missing |
| mbedtls3_ssl_context_info |  |  |  | missing |
| mbedtls3_ssl_fork_server |  |  |  | missing |
| mbedtls3_ssl_mail_client |  |  |  | missing |
| mbedtls3_ssl_pthread_server |  |  |  | missing |
| mbedtls3_ssl_server |  |  |  | missing |
| mbedtls3_ssl_server2 |  |  |  | missing |
| mbedtls3_strerror |  |  |  | missing |
| mbedtls3_udp_proxy |  |  |  | missing |
| mbedtls3_zeroize |  |  |  | missing |
| mckey | Y |  |  | covered |
| mcookie | Y |  |  | covered |
| md2html | Y |  |  | covered |
| md5sum | Y |  |  | covered |
| mdadm | Y |  |  | covered |
| mdb_copy | Y |  |  | covered |
| mdb_dump | Y |  |  | covered |
| mdb_load | Y |  |  | covered |
| mdb_stat | Y |  |  | covered |
| mdmon | Y |  |  | covered |
| media-ctl |  |  |  | missing |
| melt | Y |  |  | covered |
| melt-7 | Y |  |  | covered |
| mem_image |  |  |  | missing |
| memhog | Y |  |  | covered |
| memusage |  |  |  | missing |
| memusagestat |  |  |  | missing |
| merge_unicharsets |  |  |  | missing |
| mesg | Y |  |  | covered |
| meson | Y |  |  | covered |
| metaflac | Y |  |  | covered |
| mev | Y |  |  | covered |
| mftraining |  |  |  | missing |
| micro | Y |  | Y | covered |
| midicat | Y |  |  | covered |
| migratepages | Y |  |  | covered |
| migspeed | Y |  |  | covered |
| mise | Y |  | Y | covered |
| mk_cmds | Y |  |  | covered |
| mkdir | Y |  | Y | covered |
| mkdosfs | Y |  |  | covered |
| mke2fs | Y |  |  | covered |
| mkfifo | Y |  |  | covered |
| mkfs | Y |  |  | covered |
| mkfs.bfs | Y |  |  | covered |
| mkfs.btrfs | Y |  |  | covered |
| mkfs.cramfs | Y |  |  | covered |
| mkfs.exfat | Y |  |  | covered |
| mkfs.ext2 | Y |  |  | covered |
| mkfs.ext3 | Y |  |  | covered |
| mkfs.ext4 | Y |  |  | covered |
| mkfs.fat | Y |  |  | covered |
| mkfs.minix | Y |  |  | covered |
| mkfs.msdos | Y |  |  | covered |
| mkfs.vfat | Y |  |  | covered |
| mkhomedir_helper | Y |  |  | covered |
| mkinitcpio | Y |  | Y | covered |
| mklost+found | Y |  |  | covered |
| mknod | Y |  |  | covered |
| mkpasswd | Y |  |  | covered |
| mkrfc2734 | Y |  |  | covered |
| mksubvolume | Y |  |  | covered |
| mkswap | Y |  |  | covered |
| mktemp | Y |  | Y | covered |
| mlocate |  |  |  | missing |
| mmc-tool |  |  |  | missing |
| mmroff | Y |  |  | covered |
| modeprint |  |  |  | missing |
| modetest |  |  |  | missing |
| modinfo | Y |  |  | covered |
| modprobe | Y |  |  | covered |
| modularize |  |  |  | missing |
| modutil | Y |  |  | covered |
| mogrify | Y |  |  | covered |
| montage | Y |  |  | covered |
| moonlight |  |  |  | missing |
| more | Y |  | Y | covered |
| mount | Y |  | Y | covered |
| mount.cifs | Y |  |  | covered |
| mount.ddi | Y |  |  | covered |
| mount.fuse3 | Y |  |  | covered |
| mount.mstack | Y |  |  | covered |
| mount.smb3 | Y |  |  | covered |
| mount.storage | Y |  |  | covered |
| mountpoint | Y |  | Y | covered |
| mouse-dpi-tool | Y |  |  | covered |
| mouse-test | Y |  |  | covered |
| mp3rtp |  |  |  | missing |
| mpg123 | Y |  |  | covered |
| mpg123-id3dump |  |  |  | missing |
| mpg123-strip |  |  |  | missing |
| mpicalc |  |  |  | missing |
| mpris-proxy |  |  |  | missing |
| mpv | Y |  | Y | covered |
| mqtt5_app |  |  |  | missing |
| mqtt5_canary |  |  |  | missing |
| mqtt5canary |  |  |  | missing |
| msgattrib | Y |  |  | covered |
| msgcat | Y |  |  | covered |
| msgcmp | Y |  |  | covered |
| msgcomm | Y |  |  | covered |
| msgconv | Y |  |  | covered |
| msgen | Y |  |  | covered |
| msgexec | Y |  |  | covered |
| msgfilter | Y |  |  | covered |
| msgfmt | Y |  |  | covered |
| msggrep | Y |  |  | covered |
| msginit | Y |  |  | covered |
| msgmerge | Y |  |  | covered |
| msgpre | Y |  |  | covered |
| msgunfmt | Y |  |  | covered |
| msguniq | Y |  |  | covered |
| mtdev-test |  |  |  | missing |
| mtp-albumart |  |  |  | missing |
| mtp-albums |  |  |  | missing |
| mtp-connect |  |  |  | missing |
| mtp-delfile |  |  |  | missing |
| mtp-detect |  |  |  | missing |
| mtp-emptyfolders |  |  |  | missing |
| mtp-files |  |  |  | missing |
| mtp-filetree |  |  |  | missing |
| mtp-folders |  |  |  | missing |
| mtp-format |  |  |  | missing |
| mtp-getfile |  |  |  | missing |
| mtp-getplaylist |  |  |  | missing |
| mtp-hotplug |  |  |  | missing |
| mtp-newfolder |  |  |  | missing |
| mtp-newplaylist |  |  |  | missing |
| mtp-playlists |  |  |  | missing |
| mtp-reset |  |  |  | missing |
| mtp-sendfile |  |  |  | missing |
| mtp-sendtr |  |  |  | missing |
| mtp-thumb |  |  |  | missing |
| mtp-tracks |  |  |  | missing |
| mtp-trexist |  |  |  | missing |
| mtrace |  |  |  | missing |
| mujs |  |  |  | missing |
| mujs-pp |  |  |  | missing |
| multirender_test |  |  |  | missing |
| munch |  |  |  | missing |
| muxer |  |  |  | missing |
| mv | Y |  | Y | covered |
| mwaw2csv |  |  |  | missing |
| mwaw2html |  |  |  | missing |
| mwaw2raw |  |  |  | missing |
| mwaw2svg |  |  |  | missing |
| mwaw2text |  |  |  | missing |
| mwawFile |  |  |  | missing |
| mwawZip |  |  |  | missing |
| mysofa2json |  |  |  | missing |
| mysql_config | Y |  |  | covered |
| namei | Y |  |  | covered |
| nautilus | Y |  | Y | covered |
| nautilus-autorun-software | Y |  |  | covered |
| ncursesw6-config | Y |  |  | covered |
| ndctl | Y |  |  | covered |
| ndptool | Y |  |  | covered |
| neon-config | Y |  |  | covered |
| neqn | Y |  |  | covered |
| net | Y |  | Y | covered |
| netcap | Y |  |  | covered |
| netshaper | Y |  |  | covered |
| nettle-hash |  |  |  | missing |
| nettle-lfib-stream |  |  |  | missing |
| nettle-pbkdf2 |  |  |  | missing |
| networkctl | Y |  |  | covered |
| newgidmap | Y |  |  | covered |
| newgrp | Y |  |  | covered |
| newuidmap | Y |  |  | covered |
| newusers | Y |  |  | covered |
| nf-ct-add |  |  |  | missing |
| nf-ct-events |  |  |  | missing |
| nf-ct-list |  |  |  | missing |
| nf-exp-add |  |  |  | missing |
| nf-exp-delete |  |  |  | missing |
| nf-exp-list |  |  |  | missing |
| nf-log |  |  |  | missing |
| nf-monitor |  |  |  | missing |
| nf-queue |  |  |  | missing |
| nfbpf_compile | Y |  |  | covered |
| nfnl_osf | Y |  |  | covered |
| nfs-cat | Y |  |  | covered |
| nfs-cp | Y |  |  | covered |
| nfs-ls | Y |  |  | covered |
| nfs-stat |  |  |  | missing |
| nft | Y |  |  | covered |
| ngettext | Y |  |  | covered |
| nhlt-dmic-info | Y |  |  | covered |
| nice | Y |  | Y | covered |
| ninja |  |  |  | missing |
| nl | Y |  |  | covered |
| nl-addr-add |  |  |  | missing |
| nl-addr-delete |  |  |  | missing |
| nl-addr-list |  |  |  | missing |
| nl-class-add |  |  |  | missing |
| nl-class-delete |  |  |  | missing |
| nl-class-list |  |  |  | missing |
| nl-classid-lookup | Y |  |  | covered |
| nl-cls-add |  |  |  | missing |
| nl-cls-delete |  |  |  | missing |
| nl-cls-list |  |  |  | missing |
| nl-fib-lookup |  |  |  | missing |
| nl-link-enslave |  |  |  | missing |
| nl-link-ifindex2name |  |  |  | missing |
| nl-link-list |  |  |  | missing |
| nl-link-name2ifindex |  |  |  | missing |
| nl-link-release |  |  |  | missing |
| nl-link-set |  |  |  | missing |
| nl-link-stats |  |  |  | missing |
| nl-list-caches |  |  |  | missing |
| nl-list-sockets |  |  |  | missing |
| nl-monitor |  |  |  | missing |
| nl-neigh-add |  |  |  | missing |
| nl-neigh-delete |  |  |  | missing |
| nl-neigh-list |  |  |  | missing |
| nl-neightbl-list |  |  |  | missing |
| nl-nh-list |  |  |  | missing |
| nl-pktloc-lookup | Y |  |  | covered |
| nl-qdisc-add | Y |  |  | covered |
| nl-qdisc-delete | Y |  |  | covered |
| nl-qdisc-list | Y |  |  | covered |
| nl-route-add |  |  |  | missing |
| nl-route-delete |  |  |  | missing |
| nl-route-get |  |  |  | missing |
| nl-route-list |  |  |  | missing |
| nl-rule-list |  |  |  | missing |
| nl-tctree-list |  |  |  | missing |
| nl-util-addr |  |  |  | missing |
| nm | Y |  |  | covered |
| nm-online | Y |  |  | covered |
| nmblookup | Y |  |  | covered |
| nmcli | Y |  |  | covered |
| nmtui | Y |  |  | covered |
| nmtui-connect | Y |  |  | covered |
| nmtui-edit | Y |  |  | covered |
| nmtui-hostname | Y |  |  | covered |
| node | Y |  | Y | covered |
| nohup | Y |  | Y | covered |
| nokfw |  |  |  | missing |
| nologin | Y |  |  | covered |
| normalizer |  |  |  | missing |
| not |  |  | Y | covered |
| notify-send | Y |  |  | covered |
| nproc | Y |  | Y | covered |
| nroff | Y |  |  | covered |
| nsenter | Y |  |  | covered |
| nspr-config |  |  |  | missing |
| nss-config |  |  |  | missing |
| nstat | Y |  |  | covered |
| numactl | Y |  |  | covered |
| numademo |  |  |  | missing |
| numastat | Y |  |  | covered |
| numbers2csv |  |  |  | missing |
| numbers2raw |  |  |  | missing |
| numbers2text |  |  |  | missing |
| numfmt | Y |  |  | covered |
| nvim | Y |  | Y | covered |
| nvptx-arch |  |  |  | missing |
| obj2yaml |  |  |  | missing |
| objcopy | Y |  |  | covered |
| objdump | Y |  |  | covered |
| obs |  |  |  | missing |
| obs-ffmpeg-mux |  |  |  | missing |
| obs-nvenc-test |  |  |  | missing |
| obsidian |  |  |  | missing |
| ocsptool | Y |  |  | covered |
| od | Y |  |  | covered |
| offload-arch |  |  |  | missing |
| ojph_compress |  |  |  | missing |
| ojph_expand |  |  |  | missing |
| omacalc |  |  |  | missing |
| omacut |  |  |  | missing |
| omarchy |  |  | Y | covered |
| omarchy-agent |  |  |  | missing |
| omarchy-agent-crash |  |  |  | missing |
| omarchy-agent-prompt |  |  |  | missing |
| omarchy-agent-usage-claude |  |  |  | missing |
| omarchy-agent-usage-codex |  |  |  | missing |
| omarchy-agent-usage-fireworks |  |  |  | missing |
| omarchy-agent-usage-update |  |  |  | missing |
| omarchy-apply-hardware |  |  |  | missing |
| omarchy-apply-lock |  |  |  | missing |
| omarchy-apply-system |  |  |  | missing |
| omarchy-audio-input-mute |  |  |  | missing |
| omarchy-audio-input-set-default |  |  |  | missing |
| omarchy-audio-output-set-default |  |  |  | missing |
| omarchy-audio-output-sink |  |  |  | missing |
| omarchy-audio-output-switch |  |  |  | missing |
| omarchy-audio-output-volume |  |  |  | missing |
| omarchy-audio-sink-availability |  |  |  | missing |
| omarchy-audio-source-switch |  |  |  | missing |
| omarchy-audio-tuning |  |  |  | missing |
| omarchy-bar |  |  |  | missing |
| omarchy-bar-text-color |  |  |  | missing |
| omarchy-battery-low |  |  |  | missing |
| omarchy-battery-present |  |  |  | missing |
| omarchy-battery-status |  |  |  | missing |
| omarchy-bluetooth-device |  |  |  | missing |
| omarchy-bluetooth-power |  |  |  | missing |
| omarchy-branding-about |  |  |  | missing |
| omarchy-branding-screensaver |  |  |  | missing |
| omarchy-brightness-display |  |  |  | missing |
| omarchy-brightness-display-apple |  |  |  | missing |
| omarchy-brightness-display-ddc |  |  |  | missing |
| omarchy-brightness-keyboard |  |  |  | missing |
| omarchy-brightness-keyboard-mute |  |  |  | missing |
| omarchy-capture-qr |  |  |  | missing |
| omarchy-capture-region |  |  |  | missing |
| omarchy-capture-screenrecording |  |  |  | missing |
| omarchy-capture-screenrecording-with-webcam |  |  |  | missing |
| omarchy-capture-screenshot |  |  |  | missing |
| omarchy-capture-text |  |  |  | missing |
| omarchy-capture-webcam-list |  |  |  | missing |
| omarchy-capture-webcam-resize |  |  |  | missing |
| omarchy-channel-current |  |  |  | missing |
| omarchy-channel-set |  |  |  | missing |
| omarchy-chromium-copy-url-host |  |  |  | missing |
| omarchy-chromium-ytdlp-host |  |  |  | missing |
| omarchy-clipboard-open |  |  |  | missing |
| omarchy-clipboard-paste-file |  |  |  | missing |
| omarchy-clipboard-paste-text |  |  |  | missing |
| omarchy-cmd-missing |  |  |  | missing |
| omarchy-cmd-present |  |  |  | missing |
| omarchy-cmd-terminal-cwd |  |  |  | missing |
| omarchy-crash-watch |  |  |  | missing |
| omarchy-debug |  |  |  | missing |
| omarchy-debug-idle |  |  |  | missing |
| omarchy-default-agent |  |  |  | missing |
| omarchy-default-browser |  |  |  | missing |
| omarchy-default-editor |  |  |  | missing |
| omarchy-default-terminal |  |  |  | missing |
| omarchy-dev-add-migration |  |  |  | missing |
| omarchy-dev-benchmark-cli |  |  |  | missing |
| omarchy-dev-benchmark-theme-switcher |  |  |  | missing |
| omarchy-dev-font |  |  |  | missing |
| omarchy-dev-install-ydoo |  |  |  | missing |
| omarchy-dev-link |  |  |  | missing |
| omarchy-dev-pkg-test |  |  |  | missing |
| omarchy-dev-status |  |  |  | missing |
| omarchy-dev-theme-preview |  |  |  | missing |
| omarchy-dev-ui-preview |  |  |  | missing |
| omarchy-dev-unlink |  |  |  | missing |
| omarchy-disk-speedtest |  |  |  | missing |
| omarchy-display-text-size |  |  |  | missing |
| omarchy-dns |  |  |  | missing |
| omarchy-done |  |  |  | missing |
| omarchy-drive-info |  |  |  | missing |
| omarchy-drive-password |  |  |  | missing |
| omarchy-drive-select |  |  |  | missing |
| omarchy-file-select |  |  |  | missing |
| omarchy-font-current |  |  |  | missing |
| omarchy-font-list |  |  |  | missing |
| omarchy-font-set |  |  |  | missing |
| omarchy-games-retro-cores |  |  |  | missing |
| omarchy-games-retro-install |  |  |  | missing |
| omarchy-git-url-check |  |  |  | missing |
| omarchy-hibernation-available |  |  |  | missing |
| omarchy-hibernation-remove |  |  |  | missing |
| omarchy-hibernation-setup |  |  |  | missing |
| omarchy-hook |  |  |  | missing |
| omarchy-hook-install |  |  |  | missing |
| omarchy-hw-asus-expertbook-b9406 |  |  |  | missing |
| omarchy-hw-asus-rog |  |  |  | missing |
| omarchy-hw-asus-zenbook-ux5406aa |  |  |  | missing |
| omarchy-hw-clamshell |  |  |  | missing |
| omarchy-hw-dell-xps-haptic-touchpad |  |  |  | missing |
| omarchy-hw-dell-xps-oled |  |  |  | missing |
| omarchy-hw-display |  |  |  | missing |
| omarchy-hw-external-monitors |  |  |  | missing |
| omarchy-hw-fingerprint |  |  |  | missing |
| omarchy-hw-framework16 |  |  |  | missing |
| omarchy-hw-hybrid-gpu |  |  |  | missing |
| omarchy-hw-intel |  |  |  | missing |
| omarchy-hw-intel-ptl |  |  |  | missing |
| omarchy-hw-intel-sof |  |  |  | missing |
| omarchy-hw-laptop |  |  |  | missing |
| omarchy-hw-laptop-closed |  |  |  | missing |
| omarchy-hw-match |  |  |  | missing |
| omarchy-hw-nvidia |  |  |  | missing |
| omarchy-hw-nvidia-gsp |  |  |  | missing |
| omarchy-hw-nvidia-without-gsp |  |  |  | missing |
| omarchy-hw-recover-internal-monitor |  |  |  | missing |
| omarchy-hw-surface |  |  |  | missing |
| omarchy-hw-touchpad |  |  |  | missing |
| omarchy-hw-touchscreen |  |  |  | missing |
| omarchy-hw-vulkan |  |  |  | missing |
| omarchy-hw-webcam |  |  |  | missing |
| omarchy-hyprland-focus-app |  |  |  | missing |
| omarchy-hyprland-monitor-clamshell |  |  |  | missing |
| omarchy-hyprland-monitor-external-active |  |  |  | missing |
| omarchy-hyprland-monitor-focused |  |  |  | missing |
| omarchy-hyprland-monitor-focused-apple |  |  |  | missing |
| omarchy-hyprland-monitor-internal |  |  |  | missing |
| omarchy-hyprland-monitor-internal-mirror |  |  |  | missing |
| omarchy-hyprland-monitor-laptop |  |  |  | missing |
| omarchy-hyprland-monitor-modeless |  |  |  | missing |
| omarchy-hyprland-monitor-scaling |  |  |  | missing |
| omarchy-hyprland-monitor-watch |  |  |  | missing |
| omarchy-hyprland-reload-guard |  |  |  | missing |
| omarchy-hyprland-session-locked |  |  |  | missing |
| omarchy-hyprland-toggle |  |  |  | missing |
| omarchy-hyprland-toggle-disabled |  |  |  | missing |
| omarchy-hyprland-toggle-enabled |  |  |  | missing |
| omarchy-hyprland-window-close-all |  |  |  | missing |
| omarchy-hyprland-window-gaps-toggle |  |  |  | missing |
| omarchy-hyprland-window-pop |  |  |  | missing |
| omarchy-hyprland-window-single-square-aspect-toggle |  |  |  | missing |
| omarchy-hyprland-window-tiled-fullscreen-toggle |  |  |  | missing |
| omarchy-hyprland-window-transparency-toggle |  |  |  | missing |
| omarchy-hyprland-window-width |  |  |  | missing |
| omarchy-hyprland-workspace-layout-toggle |  |  |  | missing |
| omarchy-install-ai-chatgpt |  |  |  | missing |
| omarchy-install-ai-hermes |  |  |  | missing |
| omarchy-install-ai-openclaw |  |  |  | missing |
| omarchy-install-ai-t3-code |  |  |  | missing |
| omarchy-install-and-launch |  |  |  | missing |
| omarchy-install-app |  |  |  | missing |
| omarchy-install-browser |  |  |  | missing |
| omarchy-install-chromium-copy-url |  |  |  | missing |
| omarchy-install-chromium-google-account |  |  |  | missing |
| omarchy-install-chromium-ytdlp |  |  |  | missing |
| omarchy-install-dev-env |  |  |  | missing |
| omarchy-install-docker-dbs |  |  |  | missing |
| omarchy-install-editor-emacs |  |  |  | missing |
| omarchy-install-editor-helix |  |  |  | missing |
| omarchy-install-editor-vscode |  |  |  | missing |
| omarchy-install-editor-zed |  |  |  | missing |
| omarchy-install-font |  |  |  | missing |
| omarchy-install-gaming-battlenet |  |  |  | missing |
| omarchy-install-gaming-geforce-now |  |  |  | missing |
| omarchy-install-gaming-gpu-lib32 |  |  |  | missing |
| omarchy-install-gaming-heroic |  |  |  | missing |
| omarchy-install-gaming-lutris |  |  |  | missing |
| omarchy-install-gaming-retroarch |  |  |  | missing |
| omarchy-install-gaming-steam |  |  |  | missing |
| omarchy-install-gaming-xbox-cloud |  |  |  | missing |
| omarchy-install-gaming-xbox-controllers |  |  |  | missing |
| omarchy-install-hermes-cli |  |  |  | missing |
| omarchy-install-openclaw-cli |  |  |  | missing |
| omarchy-install-preinstalls |  |  |  | missing |
| omarchy-install-service-1password |  |  |  | missing |
| omarchy-install-service-dropbox |  |  |  | missing |
| omarchy-install-service-nordvpn |  |  |  | missing |
| omarchy-install-service-once |  |  |  | missing |
| omarchy-install-service-signal |  |  |  | missing |
| omarchy-install-service-spotify |  |  |  | missing |
| omarchy-install-service-sunshine |  |  |  | missing |
| omarchy-install-service-tailscale |  |  |  | missing |
| omarchy-install-terminal |  |  |  | missing |
| omarchy-installed-service-dropbox |  |  |  | missing |
| omarchy-installed-service-tailscale |  |  |  | missing |
| omarchy-launch-1password |  |  |  | missing |
| omarchy-launch-about |  |  |  | missing |
| omarchy-launch-battlenet |  |  |  | missing |
| omarchy-launch-browser |  |  |  | missing |
| omarchy-launch-config-editor |  |  |  | missing |
| omarchy-launch-discord-community |  |  |  | missing |
| omarchy-launch-docker-tui |  |  |  | missing |
| omarchy-launch-editor |  |  |  | missing |
| omarchy-launch-floating-terminal-with-presentation |  |  |  | missing |
| omarchy-launch-nautilus |  |  |  | missing |
| omarchy-launch-nautilus-cwd |  |  |  | missing |
| omarchy-launch-openclaw |  |  |  | missing |
| omarchy-launch-or-focus |  |  |  | missing |
| omarchy-launch-or-focus-tui |  |  |  | missing |
| omarchy-launch-or-focus-webapp |  |  |  | missing |
| omarchy-launch-screensaver |  |  |  | missing |
| omarchy-launch-shell |  |  |  | missing |
| omarchy-launch-signal |  |  |  | missing |
| omarchy-launch-spotify |  |  |  | missing |
| omarchy-launch-terminal |  |  |  | missing |
| omarchy-launch-terminal-herdr |  |  |  | missing |
| omarchy-launch-terminal-tmux |  |  |  | missing |
| omarchy-launch-tui |  |  |  | missing |
| omarchy-launch-webapp |  |  |  | missing |
| omarchy-menu |  |  |  | missing |
| omarchy-menu-clipboard |  |  |  | missing |
| omarchy-menu-emoji |  |  |  | missing |
| omarchy-menu-emoji-insert |  |  |  | missing |
| omarchy-menu-file |  |  |  | missing |
| omarchy-menu-herdr-keybindings |  |  |  | missing |
| omarchy-menu-images |  |  |  | missing |
| omarchy-menu-input |  |  |  | missing |
| omarchy-menu-keybindings |  |  |  | missing |
| omarchy-menu-plugin |  |  |  | missing |
| omarchy-menu-select |  |  |  | missing |
| omarchy-menu-share |  |  |  | missing |
| omarchy-menu-timezone |  |  |  | missing |
| omarchy-menu-tmux-keybindings |  |  |  | missing |
| omarchy-migrate |  |  |  | missing |
| omarchy-migrate-notify |  |  |  | missing |
| omarchy-mise-install |  |  |  | missing |
| omarchy-monitor-state |  |  |  | missing |
| omarchy-network-band |  |  |  | missing |
| omarchy-network-password |  |  |  | missing |
| omarchy-network-qr |  |  |  | missing |
| omarchy-network-speedtest |  |  |  | missing |
| omarchy-network-status |  |  |  | missing |
| omarchy-notification-battery |  |  |  | missing |
| omarchy-notification-dismiss |  |  |  | missing |
| omarchy-notification-send |  |  |  | missing |
| omarchy-notification-time |  |  |  | missing |
| omarchy-notification-wait |  |  |  | missing |
| omarchy-notification-weather |  |  |  | missing |
| omarchy-nvim-refresh |  |  |  | missing |
| omarchy-nvim-setup |  |  |  | missing |
| omarchy-openclaw-onboard |  |  |  | missing |
| omarchy-osd |  |  |  | missing |
| omarchy-pkg-add |  |  |  | missing |
| omarchy-pkg-aur-accessible |  |  |  | missing |
| omarchy-pkg-aur-add |  |  |  | missing |
| omarchy-pkg-aur-install |  |  |  | missing |
| omarchy-pkg-drop |  |  |  | missing |
| omarchy-pkg-install |  |  |  | missing |
| omarchy-pkg-missing |  |  |  | missing |
| omarchy-pkg-present |  |  |  | missing |
| omarchy-pkg-remove |  |  |  | missing |
| omarchy-plugin-add |  |  |  | missing |
| omarchy-plugin-catalog |  |  |  | missing |
| omarchy-plugin-clone |  |  |  | missing |
| omarchy-plugin-disable |  |  |  | missing |
| omarchy-plugin-enable |  |  |  | missing |
| omarchy-plugin-list |  |  |  | missing |
| omarchy-plugin-remove |  |  |  | missing |
| omarchy-plugin-update |  |  |  | missing |
| omarchy-plugin-validate |  |  |  | missing |
| omarchy-plymouth-current |  |  |  | missing |
| omarchy-plymouth-list |  |  |  | missing |
| omarchy-plymouth-preview |  |  |  | missing |
| omarchy-plymouth-reset |  |  |  | missing |
| omarchy-plymouth-set |  |  |  | missing |
| omarchy-plymouth-set-by-theme |  |  |  | missing |
| omarchy-plymouth-switcher |  |  |  | missing |
| omarchy-power-present |  |  |  | missing |
| omarchy-powerprofiles-init |  |  |  | missing |
| omarchy-powerprofiles-list |  |  |  | missing |
| omarchy-powerprofiles-set |  |  |  | missing |
| omarchy-provision-first-run |  |  |  | missing |
| omarchy-provision-owner |  |  |  | missing |
| omarchy-provision-user |  |  |  | missing |
| omarchy-refresh-applications |  |  |  | missing |
| omarchy-refresh-chromium |  |  |  | missing |
| omarchy-refresh-config |  |  |  | missing |
| omarchy-refresh-herdr |  |  |  | missing |
| omarchy-refresh-hyprland |  |  |  | missing |
| omarchy-refresh-hyprsunset |  |  |  | missing |
| omarchy-refresh-limine |  |  |  | missing |
| omarchy-refresh-pacman |  |  |  | missing |
| omarchy-refresh-plymouth |  |  |  | missing |
| omarchy-refresh-sddm |  |  |  | missing |
| omarchy-refresh-shell |  |  |  | missing |
| omarchy-refresh-tmux |  |  |  | missing |
| omarchy-reinstall |  |  |  | missing |
| omarchy-reinstall-configs |  |  |  | missing |
| omarchy-reinstall-pkgs |  |  |  | missing |
| omarchy-reminder |  |  |  | missing |
| omarchy-remove-ai-hermes |  |  |  | missing |
| omarchy-remove-ai-openclaw |  |  |  | missing |
| omarchy-remove-ai-perplexity |  |  |  | missing |
| omarchy-remove-ai-t3-code |  |  |  | missing |
| omarchy-remove-browser |  |  |  | missing |
| omarchy-remove-dev-env |  |  |  | missing |
| omarchy-remove-gaming-battlenet |  |  |  | missing |
| omarchy-remove-gaming-geforce-now |  |  |  | missing |
| omarchy-remove-gaming-heroic |  |  |  | missing |
| omarchy-remove-gaming-lutris |  |  |  | missing |
| omarchy-remove-gaming-minecraft |  |  |  | missing |
| omarchy-remove-gaming-retroarch |  |  |  | missing |
| omarchy-remove-gaming-steam |  |  |  | missing |
| omarchy-remove-gaming-xbox-cloud |  |  |  | missing |
| omarchy-remove-gaming-xbox-controllers |  |  |  | missing |
| omarchy-remove-launcher-entry |  |  |  | missing |
| omarchy-remove-preinstalls |  |  |  | missing |
| omarchy-remove-security-fido2 |  |  |  | missing |
| omarchy-remove-security-fingerprint |  |  |  | missing |
| omarchy-remove-security-sshd |  |  |  | missing |
| omarchy-remove-security-sudoless-docker |  |  |  | missing |
| omarchy-remove-service-1password |  |  |  | missing |
| omarchy-remove-service-dropbox |  |  |  | missing |
| omarchy-remove-service-sunshine |  |  |  | missing |
| omarchy-remove-service-tailscale |  |  |  | missing |
| omarchy-restart-app |  |  |  | missing |
| omarchy-restart-audio |  |  |  | missing |
| omarchy-restart-bluetooth |  |  |  | missing |
| omarchy-restart-btop |  |  |  | missing |
| omarchy-restart-gum |  |  |  | missing |
| omarchy-restart-helix |  |  |  | missing |
| omarchy-restart-herdr |  |  |  | missing |
| omarchy-restart-hyprctl |  |  |  | missing |
| omarchy-restart-hyprsunset |  |  |  | missing |
| omarchy-restart-opencode |  |  |  | missing |
| omarchy-restart-shell |  |  |  | missing |
| omarchy-restart-terminal |  |  |  | missing |
| omarchy-restart-tmux |  |  |  | missing |
| omarchy-restart-trackpad |  |  |  | missing |
| omarchy-restart-wifi |  |  |  | missing |
| omarchy-restart-xcompose |  |  |  | missing |
| omarchy-screensaver |  |  |  | missing |
| omarchy-setup-direct-boot |  |  |  | missing |
| omarchy-setup-security-fido2 |  |  |  | missing |
| omarchy-setup-security-fingerprint |  |  |  | missing |
| omarchy-setup-security-sshd |  |  |  | missing |
| omarchy-setup-security-sudoless-docker |  |  |  | missing |
| omarchy-shell |  |  |  | missing |
| omarchy-shell-config |  |  |  | missing |
| omarchy-show-done |  |  |  | missing |
| omarchy-show-logo |  |  |  | missing |
| omarchy-snapshot |  |  |  | missing |
| omarchy-state |  |  |  | missing |
| omarchy-sudo-docker |  |  |  | missing |
| omarchy-sudo-keepalive |  |  |  | missing |
| omarchy-sudo-passwordless |  |  |  | missing |
| omarchy-system-factory-reset |  |  |  | missing |
| omarchy-system-factory-reset-finish |  |  |  | missing |
| omarchy-system-lid-close |  |  |  | missing |
| omarchy-system-lock |  |  |  | missing |
| omarchy-system-logout |  |  |  | missing |
| omarchy-system-reboot |  |  |  | missing |
| omarchy-system-shutdown |  |  |  | missing |
| omarchy-system-sleep-lock |  |  |  | missing |
| omarchy-system-sleep-monitor |  |  |  | missing |
| omarchy-system-stats |  |  |  | missing |
| omarchy-system-wake |  |  |  | missing |
| omarchy-tailscale-receive |  |  |  | missing |
| omarchy-tailscale-send |  |  |  | missing |
| omarchy-theme-bg-cache |  |  |  | missing |
| omarchy-theme-bg-current |  |  |  | missing |
| omarchy-theme-bg-install |  |  |  | missing |
| omarchy-theme-bg-next |  |  |  | missing |
| omarchy-theme-bg-set |  |  |  | missing |
| omarchy-theme-bg-switcher |  |  |  | missing |
| omarchy-theme-color |  |  |  | missing |
| omarchy-theme-colors-from-alacritty |  |  |  | missing |
| omarchy-theme-current |  |  |  | missing |
| omarchy-theme-dir |  |  |  | missing |
| omarchy-theme-extras |  |  |  | missing |
| omarchy-theme-install |  |  |  | missing |
| omarchy-theme-list |  |  |  | missing |
| omarchy-theme-osc |  |  |  | missing |
| omarchy-theme-refresh |  |  |  | missing |
| omarchy-theme-remove |  |  |  | missing |
| omarchy-theme-set |  |  |  | missing |
| omarchy-theme-set-browser |  |  |  | missing |
| omarchy-theme-set-browser-policy |  |  |  | missing |
| omarchy-theme-set-claude |  |  |  | missing |
| omarchy-theme-set-foot |  |  |  | missing |
| omarchy-theme-set-gnome |  |  |  | missing |
| omarchy-theme-set-hermes |  |  |  | missing |
| omarchy-theme-set-keyboard |  |  |  | missing |
| omarchy-theme-set-keyboard-asus-rog |  |  |  | missing |
| omarchy-theme-set-keyboard-f16 |  |  |  | missing |
| omarchy-theme-set-obsidian |  |  |  | missing |
| omarchy-theme-set-pi |  |  |  | missing |
| omarchy-theme-set-t3code |  |  |  | missing |
| omarchy-theme-set-templates |  |  |  | missing |
| omarchy-theme-set-tmux |  |  |  | missing |
| omarchy-theme-set-vscode |  |  |  | missing |
| omarchy-theme-switcher |  |  |  | missing |
| omarchy-theme-update |  |  |  | missing |
| omarchy-toggle |  |  |  | missing |
| omarchy-toggle-bar |  |  |  | missing |
| omarchy-toggle-crash-capture |  |  |  | missing |
| omarchy-toggle-enabled |  |  |  | missing |
| omarchy-toggle-hybrid-gpu |  |  |  | missing |
| omarchy-toggle-idle |  |  |  | missing |
| omarchy-toggle-input-device |  |  |  | missing |
| omarchy-toggle-nightlight |  |  |  | missing |
| omarchy-toggle-notification-silencing |  |  |  | missing |
| omarchy-toggle-screensaver |  |  |  | missing |
| omarchy-toggle-suspend |  |  |  | missing |
| omarchy-toggle-touchpad |  |  |  | missing |
| omarchy-toggle-touchscreen |  |  |  | missing |
| omarchy-transcode |  |  |  | missing |
| omarchy-transcode-ascii |  |  |  | missing |
| omarchy-tui-install |  |  |  | missing |
| omarchy-tui-remove |  |  |  | missing |
| omarchy-tui-remove-all |  |  |  | missing |
| omarchy-update |  |  |  | missing |
| omarchy-update-analyze-logs |  |  |  | missing |
| omarchy-update-aur-pkgs |  |  |  | missing |
| omarchy-update-available |  |  |  | missing |
| omarchy-update-confirm |  |  |  | missing |
| omarchy-update-dev |  |  |  | missing |
| omarchy-update-firmware |  |  |  | missing |
| omarchy-update-keyring |  |  |  | missing |
| omarchy-update-lock |  |  |  | missing |
| omarchy-update-mise |  |  |  | missing |
| omarchy-update-orphan-pkgs |  |  |  | missing |
| omarchy-update-pacman-guard |  |  |  | missing |
| omarchy-update-pkg-prune |  |  |  | missing |
| omarchy-update-requires-free-space |  |  |  | missing |
| omarchy-update-restart |  |  |  | missing |
| omarchy-update-status |  |  |  | missing |
| omarchy-update-stay-awake |  |  |  | missing |
| omarchy-update-system-pkgs |  |  |  | missing |
| omarchy-update-system-pkgs-when-conflicted |  |  |  | missing |
| omarchy-update-time |  |  |  | missing |
| omarchy-update-user-notify |  |  |  | missing |
| omarchy-upgrade-to-quattro |  |  |  | missing |
| omarchy-upload-log |  |  |  | missing |
| omarchy-version |  |  |  | missing |
| omarchy-version-branch |  |  |  | missing |
| omarchy-version-channel |  |  |  | missing |
| omarchy-version-pkgs |  |  |  | missing |
| omarchy-voxtype-config |  |  |  | missing |
| omarchy-voxtype-install |  |  |  | missing |
| omarchy-voxtype-model |  |  |  | missing |
| omarchy-voxtype-remove |  |  |  | missing |
| omarchy-voxtype-status |  |  |  | missing |
| omarchy-weather-icon |  |  |  | missing |
| omarchy-weather-location |  |  |  | missing |
| omarchy-weather-status |  |  |  | missing |
| omarchy-webapp-handler-hey |  |  |  | missing |
| omarchy-webapp-handler-zoom |  |  |  | missing |
| omarchy-webapp-install |  |  |  | missing |
| omarchy-webapp-remove |  |  |  | missing |
| omarchy-webapp-remove-all |  |  |  | missing |
| omarchy-windows-vm |  |  |  | missing |
| onig-config |  |  |  | missing |
| oobtest |  |  |  | missing |
| oomctl | Y |  |  | covered |
| openal-info |  |  |  | missing |
| opencv_annotation |  |  |  | missing |
| opencv_model_diagnostics |  |  |  | missing |
| opencv_version |  |  |  | missing |
| opencv_visualisation |  |  |  | missing |
| opencv_waldboost_detector |  |  |  | missing |
| openmpt123 | Y |  |  | covered |
| openssl | Y |  |  | covered |
| openvt | Y |  |  | covered |
| opj_compress | Y |  |  | covered |
| opj_decompress | Y |  |  | covered |
| opj_dump | Y |  |  | covered |
| opt | Y |  |  | covered |
| orc-bugreport |  |  |  | missing |
| orcc |  |  |  | missing |
| orcus-css-dump |  |  |  | missing |
| orcus-csv |  |  |  | missing |
| orcus-detect |  |  |  | missing |
| orcus-gnumeric |  |  |  | missing |
| orcus-json |  |  |  | missing |
| orcus-mso-encryption |  |  |  | missing |
| orcus-ods |  |  |  | missing |
| orcus-styles-ods |  |  |  | missing |
| orcus-xls-xml |  |  |  | missing |
| orcus-xlsx |  |  |  | missing |
| orcus-xml |  |  |  | missing |
| orcus-yaml |  |  |  | missing |
| orcus-zip-dump |  |  |  | missing |
| osinfo-detect | Y |  |  | covered |
| osinfo-install-script | Y |  |  | covered |
| osinfo-query | Y |  |  | covered |
| otioautogen_serialized_schema_docs |  |  |  | missing |
| otiocat |  |  |  | missing |
| otioconvert |  |  |  | missing |
| otiopluginfo |  |  |  | missing |
| otiostat |  |  |  | missing |
| otiotool |  |  |  | missing |
| out123 | Y |  |  | covered |
| outpsfheader |  |  |  | missing |
| p11-kit | Y |  |  | covered |
| p11tool | Y |  |  | covered |
| pacat | Y |  |  | covered |
| paccache | Y |  |  | covered |
| pacdiff | Y |  |  | covered |
| paclist | Y |  |  | covered |
| paclog-pkglist | Y |  |  | covered |
| pacman | Y |  | Y | covered |
| pacman-conf | Y |  |  | covered |
| pacman-db-upgrade | Y |  |  | covered |
| pacman-key | Y |  |  | covered |
| pacscripts | Y |  |  | covered |
| pacsearch | Y |  |  | covered |
| pacsort | Y |  |  | covered |
| pactl | Y |  |  | covered |
| pactree | Y |  |  | covered |
| padsp | Y |  |  | covered |
| pages2html |  |  |  | missing |
| pages2raw |  |  |  | missing |
| pages2text |  |  |  | missing |
| pahole | Y |  |  | covered |
| pal2rgb |  |  |  | missing |
| pam_namespace_helper | Y |  |  | covered |
| pam_timestamp_check | Y |  |  | covered |
| pamixer | Y |  |  | covered |
| pamon | Y |  |  | covered |
| panelctl | Y |  |  | covered |
| pango-list | Y |  |  | covered |
| pango-segmentation | Y |  |  | covered |
| pango-view | Y |  |  | covered |
| paper | Y |  |  | covered |
| paperconf |  |  |  | missing |
| paplay | Y |  |  | covered |
| parec | Y |  |  | covered |
| parecord | Y |  |  | covered |
| parse_companies.pl |  |  |  | missing |
| parsetrigrams6 |  |  |  | missing |
| parted | Y | Y |  | covered |
| partprobe | Y |  |  | covered |
| partx | Y |  |  | covered |
| passwd | Y |  | Y | covered |
| paste | Y |  | Y | covered |
| patch | Y |  | Y | covered |
| pathchk | Y |  |  | covered |
| pbmtojbg | Y |  |  | covered |
| pbmtojbg85 |  |  |  | missing |
| pcap-config | Y |  |  | covered |
| pccritic | Y |  |  | covered |
| pcilmr | Y |  |  | covered |
| pcprofiledump |  |  |  | missing |
| pcre-config | Y |  |  | covered |
| pcre2-config | Y |  |  | covered |
| pcre2grep | Y |  |  | covered |
| pcre2test | Y |  |  | covered |
| pcregrep | Y |  |  | covered |
| pcretest | Y |  |  | covered |
| pcsc-spy | Y |  |  | covered |
| pcscd | Y |  |  | covered |
| pdf2dsc |  |  |  | missing |
| pdf2ps | Y |  |  | covered |
| pdfattach | Y |  |  | covered |
| pdfdetach | Y |  |  | covered |
| pdffonts | Y |  |  | covered |
| pdfimages | Y |  |  | covered |
| pdfinfo | Y |  |  | covered |
| pdfmom | Y |  |  | covered |
| pdfseparate | Y |  |  | covered |
| pdfsig | Y |  |  | covered |
| pdftexi2dvi | Y |  |  | covered |
| pdftocairo | Y |  |  | covered |
| pdftohtml | Y |  |  | covered |
| pdftoppm | Y |  |  | covered |
| pdftops | Y |  |  | covered |
| pdftotext | Y |  |  | covered |
| pdfunite | Y |  |  | covered |
| pdwtags |  |  |  | missing |
| peekfd | Y |  |  | covered |
| perfquery | Y |  |  | covered |
| perl | Y |  |  | covered |
| pf2afm | Y |  |  | covered |
| pfbtopfa | Y |  |  | covered |
| pfbtops | Y |  |  | covered |
| pfunct | Y |  |  | covered |
| pg | Y |  |  | covered |
| pg_config | Y |  |  | covered |
| pg_dump | Y |  |  | covered |
| pg_dumpall | Y |  |  | covered |
| pg_isready | Y |  |  | covered |
| pg_restore | Y |  |  | covered |
| pglobal |  |  |  | missing |
| pgrep | Y |  | Y | covered |
| pic | Y |  |  | covered |
| pic2graph | Y |  |  | covered |
| pidof | Y |  |  | covered |
| pidwait | Y |  |  | covered |
| pinentry |  | Y |  | covered |
| pinentry-curses |  |  |  | missing |
| pinentry-emacs |  |  |  | missing |
| pinentry-gnome3 |  |  |  | missing |
| pinentry-gtk |  |  |  | missing |
| pinentry-qt |  |  |  | missing |
| pinentry-tty |  |  |  | missing |
| ping | Y |  |  | covered |
| pinky | Y |  |  | covered |
| pinta | Y |  |  | covered |
| pipesz | Y |  |  | covered |
| pipewire | Y |  |  | covered |
| pipewire-aes67 |  |  |  | missing |
| pipewire-avb |  |  |  | missing |
| pipewire-pulse | Y |  |  | covered |
| pivot_root | Y |  |  | covered |
| pk-example-frobnicate |  |  |  | missing |
| pk12util | Y |  |  | covered |
| pkaction | Y |  |  | covered |
| pkcheck | Y |  |  | covered |
| pkcs1-conv |  |  |  | missing |
| pkexec | Y |  | Y | covered |
| pkg-config | Y |  |  | covered |
| pkgconf | Y |  |  | covered |
| pkgdata | Y |  |  | covered |
| pkill | Y |  | Y | covered |
| pkttyagent | Y |  |  | covered |
| pldd |  |  |  | missing |
| plistutil | Y |  |  | covered |
| plocate | Y |  |  | covered |
| plocate-build | Y |  |  | covered |
| plugctl | Y |  |  | covered |
| pluginviewer | Y |  |  | covered |
| plugreport | Y |  |  | covered |
| plymouth | Y |  |  | covered |
| plymouth-set-default-theme | Y |  |  | covered |
| plymouthd | Y |  |  | covered |
| pmap | Y |  |  | covered |
| pmd2raw |  |  |  | missing |
| pmd2svg |  |  |  | missing |
| pmd2text |  |  |  | missing |
| png-fix-itxt |  |  |  | missing |
| png2pnm |  |  |  | missing |
| pngfix |  |  |  | missing |
| pnm2png |  |  |  | missing |
| po-fetch | Y |  |  | covered |
| pod2texi | Y |  |  | covered |
| portablectl | Y |  |  | covered |
| post-grohtml | Y |  |  | covered |
| postprocessing_benchmark |  |  |  | missing |
| poweroff | Y |  |  | covered |
| powerprofilesctl | Y |  |  | covered |
| pp-trace |  |  |  | missing |
| ppdc | Y |  |  | covered |
| ppdhtml | Y |  |  | covered |
| ppdi | Y |  |  | covered |
| ppdmerge | Y |  |  | covered |
| ppdpo | Y |  |  | covered |
| pphs |  |  |  | missing |
| ppm2tiff |  |  |  | missing |
| pr | Y |  |  | covered |
| pre-grohtml | Y |  |  | covered |
| preconv | Y |  |  | covered |
| prefcnt |  |  |  | missing |
| printafm | Y |  |  | covered |
| printenv | Y |  |  | covered |
| printf | Y |  | Y | covered |
| printf_gettext | Y |  |  | covered |
| printf_ngettext | Y |  |  | covered |
| prlimit | Y |  |  | covered |
| procan |  |  |  | missing |
| proptest |  |  |  | missing |
| protoc |  |  |  | missing |
| protoc-36.1.0 |  |  |  | missing |
| protoc-gen-upb |  |  |  | missing |
| protoc-gen-upb-36.1.0 |  |  |  | missing |
| protoc-gen-upb_minitable |  |  |  | missing |
| protoc-gen-upb_minitable-36.1.0 |  |  |  | missing |
| protoc-gen-upbdefs |  |  |  | missing |
| protoc-gen-upbdefs-36.1.0 |  |  |  | missing |
| proxy | Y |  |  | covered |
| proxy_thr |  |  |  | missing |
| prtstat | Y |  |  | covered |
| ps | Y |  | Y | covered |
| ps2ascii | Y |  |  | covered |
| ps2epsi | Y |  |  | covered |
| ps2pdf | Y |  |  | covered |
| ps2pdf12 | Y |  |  | covered |
| ps2pdf13 | Y |  |  | covered |
| ps2pdf14 | Y |  |  | covered |
| ps2pdfwr | Y |  |  | covered |
| ps2ps | Y |  |  | covered |
| ps2ps2 |  |  |  | missing |
| pscap | Y |  |  | covered |
| psfaddtable | Y |  |  | covered |
| psfgettable | Y |  |  | covered |
| psfstriptable | Y |  |  | covered |
| psfxtable | Y |  |  | covered |
| psicc | Y |  |  | covered |
| psktool | Y |  |  | covered |
| psl | Y |  |  | covered |
| psl-make-dafsa | Y |  |  | covered |
| pslog | Y |  |  | covered |
| psql | Y |  |  | covered |
| pstree | Y |  |  | covered |
| pstree.x11 |  |  |  | missing |
| ptx | Y |  |  | covered |
| pub2raw |  |  |  | missing |
| pub2xhtml |  |  |  | missing |
| pw-cat | Y |  |  | covered |
| pw-cli | Y |  |  | covered |
| pw-config | Y |  |  | covered |
| pw-container | Y |  |  | covered |
| pw-dot | Y |  |  | covered |
| pw-dsdplay |  |  |  | missing |
| pw-dump | Y |  |  | covered |
| pw-encplay |  |  |  | missing |
| pw-jack | Y |  |  | covered |
| pw-link | Y |  |  | covered |
| pw-loopback | Y |  |  | covered |
| pw-metadata | Y |  |  | covered |
| pw-midi2play |  |  |  | missing |
| pw-midi2record |  |  |  | missing |
| pw-mididump | Y |  |  | covered |
| pw-midiplay |  |  |  | missing |
| pw-midirecord |  |  |  | missing |
| pw-mon | Y |  |  | covered |
| pw-play |  |  |  | missing |
| pw-profiler | Y |  |  | covered |
| pw-record |  |  |  | missing |
| pw-reserve | Y |  |  | covered |
| pw-sysex |  |  |  | missing |
| pw-top | Y |  |  | covered |
| pwck | Y |  |  | covered |
| pwconv | Y |  |  | covered |
| pwd | Y |  |  | covered |
| pwdx | Y |  |  | covered |
| pwhistory_helper |  |  |  | missing |
| pwmake | Y |  |  | covered |
| pwmconfig | Y |  |  | covered |
| pwscore | Y |  |  | covered |
| pwunconv | Y |  |  | covered |
| pydoc |  |  |  | missing |
| pydoc3 |  |  |  | missing |
| pydoc3.14 |  |  |  | missing |
| python | Y |  | Y | covered |
| python-config |  |  |  | missing |
| python3 | Y |  | Y | covered |
| python3-config |  |  |  | missing |
| python3.14 | Y |  |  | covered |
| python3.14-config |  |  |  | missing |
| pzstd |  |  |  | missing |
| qemu-aarch64-static |  |  |  | missing |
| qemu-aarch64_be-static |  |  |  | missing |
| qemu-alpha-static |  |  |  | missing |
| qemu-arm-static |  |  |  | missing |
| qemu-armeb-static |  |  |  | missing |
| qemu-hexagon-static |  |  |  | missing |
| qemu-hppa-static |  |  |  | missing |
| qemu-i386-static |  |  |  | missing |
| qemu-loongarch64-static |  |  |  | missing |
| qemu-m68k-static |  |  |  | missing |
| qemu-microblaze-static |  |  |  | missing |
| qemu-microblazeel-static |  |  |  | missing |
| qemu-mips-static |  |  |  | missing |
| qemu-mips64-static |  |  |  | missing |
| qemu-mips64el-static |  |  |  | missing |
| qemu-mipsel-static |  |  |  | missing |
| qemu-mipsn32-static |  |  |  | missing |
| qemu-mipsn32el-static |  |  |  | missing |
| qemu-or1k-static |  |  |  | missing |
| qemu-ppc-static |  |  |  | missing |
| qemu-ppc64-static |  |  |  | missing |
| qemu-ppc64le-static |  |  |  | missing |
| qemu-riscv32-static |  |  |  | missing |
| qemu-riscv64-static |  |  |  | missing |
| qemu-s390x-static |  |  |  | missing |
| qemu-sh4-static |  |  |  | missing |
| qemu-sh4eb-static |  |  |  | missing |
| qemu-sparc-static |  |  |  | missing |
| qemu-sparc32plus-static |  |  |  | missing |
| qemu-sparc64-static |  |  |  | missing |
| qemu-system-i386 | Y |  |  | covered |
| qemu-system-x86_64 | Y |  |  | covered |
| qemu-x86_64-static |  |  |  | missing |
| qemu-xtensa-static |  |  |  | missing |
| qemu-xtensaeb-static |  |  |  | missing |
| qmake6 |  |  |  | missing |
| qml6 |  |  |  | missing |
| qmleasing6 |  |  |  | missing |
| qmlls6 |  |  |  | missing |
| qmlpreview6 |  |  |  | missing |
| qmlscene6 |  |  |  | missing |
| qpdf | Y |  |  | covered |
| qrencode | Y |  |  | covered |
| qs |  |  |  | missing |
| qt-faststart |  |  |  | missing |
| quickshell |  |  | Y | covered |
| qv4l2 | Y |  |  | covered |
| qvidcap | Y |  |  | covered |
| qxp2raw |  |  |  | missing |
| qxp2svg |  |  |  | missing |
| qxp2text |  |  |  | missing |
| raid6check | Y |  |  | covered |
| rankmirrors | Y |  |  | covered |
| ranlib | Y |  |  | covered |
| rapper | Y |  |  | covered |
| rasqal-config | Y |  |  | covered |
| rav1e |  |  |  | missing |
| raw-identify |  |  |  | missing |
| raw2tiff |  |  |  | missing |
| rawtextdump |  |  |  | missing |
| rbash |  |  |  | missing |
| rcopy | Y |  |  | covered |
| rcp | Y |  |  | covered |
| rctest | Y |  |  | covered |
| rdfproc | Y |  |  | covered |
| rdjpgcom | Y |  |  | covered |
| rdma | Y |  |  | covered |
| rdma-ndd | Y |  |  | covered |
| rdma_client | Y |  |  | covered |
| rdma_server | Y |  |  | covered |
| rdma_topo |  |  |  | missing |
| rdma_xclient | Y |  |  | covered |
| rdma_xserver | Y |  |  | covered |
| rds-ctl |  |  |  | missing |
| readelf | Y |  | Y | covered |
| readlink | Y |  |  | covered |
| readprofile | Y |  |  | covered |
| realpath | Y |  |  | covered |
| reboot | Y |  | Y | covered |
| recode-sr-latin | Y |  |  | covered |
| redland-config | Y |  |  | covered |
| redland-db-upgrade | Y |  |  | covered |
| reduce-chunk-list |  |  |  | missing |
| refer | Y |  |  | covered |
| reindexdb | Y |  |  | covered |
| remote_lat |  |  |  | missing |
| remote_thr |  |  |  | missing |
| remuxer |  |  |  | missing |
| rename | Y |  | Y | covered |
| renice | Y |  |  | covered |
| repo-add | Y |  |  | covered |
| repo-elephant |  |  |  | missing |
| repo-remove | Y |  |  | covered |
| request-key | Y |  |  | covered |
| reset | Y |  | Y | covered |
| resize2fs | Y |  |  | covered |
| resizecons | Y |  |  | covered |
| resizepart | Y |  |  | covered |
| resolvectl | Y |  |  | covered |
| rev | Y |  |  | covered |
| rfkill | Y |  |  | covered |
| rg | Y |  |  | covered |
| rhash | Y |  |  | covered |
| riostream | Y |  |  | covered |
| rlogin | Y |  |  | covered |
| rlogind | Y |  |  | covered |
| rm | Y |  | Y | covered |
| rmdir | Y |  |  | covered |
| rmmod | Y |  |  | covered |
| roqet | Y |  |  | covered |
| routel | Y |  |  | covered |
| rpcclient | Y |  |  | covered |
| rping | Y |  |  | covered |
| rrsync | Y |  |  | covered |
| rsh | Y |  |  | covered |
| rshd | Y |  |  | covered |
| rstream | Y |  |  | covered |
| rsvg-convert | Y |  |  | covered |
| rsync | Y |  |  | covered |
| rsync-ssl | Y |  |  | covered |
| rtacct | Y |  |  | covered |
| rtcwake | Y |  |  | covered |
| rtlfw |  |  |  | missing |
| rtmon | Y |  |  | covered |
| rtstat | Y |  |  | covered |
| rubberband |  |  |  | missing |
| rubberband-r3 |  |  |  | missing |
| ruby | Y |  |  | covered |
| run-clang-tidy |  |  |  | missing |
| run0 | Y |  |  | covered |
| run_srp_daemon |  |  |  | missing |
| runc | Y |  |  | covered |
| runuser | Y |  |  | covered |
| s3 |  |  | Y | covered |
| sancov |  |  |  | missing |
| sanstats |  |  |  | missing |
| saquery | Y |  |  | covered |
| sasldblistusers2 | Y |  |  | covered |
| saslpasswd2 | Y |  |  | covered |
| sbcdec |  |  |  | missing |
| sbcenc |  |  |  | missing |
| sbcinfo |  |  |  | missing |
| scalar | Y |  | Y | covered |
| scan-build | Y |  |  | covered |
| scan-build-py |  |  |  | missing |
| scan-view |  |  |  | missing |
| sclient | Y |  |  | covered |
| scmp_sys_resolver | Y |  |  | covered |
| scncopy |  |  |  | missing |
| scotest |  |  |  | missing |
| scp | Y |  |  | covered |
| scp-dbus-service |  |  |  | missing |
| screendump | Y |  |  | covered |
| script | Y |  | Y | covered |
| scriptlive | Y |  |  | covered |
| scriptreplay | Y |  |  | covered |
| sd2raw |  |  |  | missing |
| sd2svg |  |  |  | missing |
| sd2text |  |  |  | missing |
| sdbus-c++-xml2cpp |  |  |  | missing |
| sdc2csv |  |  |  | missing |
| sddm | Y |  | Y | covered |
| sddm-greeter | Y |  |  | covered |
| sddm-greeter-qt6 |  |  | Y | covered |
| sdiff | Y |  |  | covered |
| sdl2-config |  |  |  | missing |
| sdw2html |  |  |  | missing |
| seatd | Y |  | Y | covered |
| seatd-launch | Y |  |  | covered |
| secret-tool | Y |  |  | covered |
| sed | Y | Y |  | covered |
| sendiso | Y |  |  | covered |
| sensord | Y |  |  | covered |
| sensors | Y |  |  | covered |
| sensors-conf-convert | Y |  |  | covered |
| sensors-detect | Y |  |  | covered |
| sepdebugcrcfix | Y |  |  | covered |
| seq | Y |  |  | covered |
| seq2bseq |  |  |  | missing |
| serdi | Y |  |  | covered |
| set-wireless-regdom |  |  |  | missing |
| set_unicharset_properties |  |  |  | missing |
| setarch | Y |  |  | covered |
| setcap | Y |  |  | covered |
| setcifsacl | Y |  |  | covered |
| setfacl | Y |  |  | covered |
| setfattr | Y |  |  | covered |
| setfont | Y |  | Y | covered |
| setkeycodes | Y |  |  | covered |
| setleds | Y |  |  | covered |
| setlogcons | Y |  |  | covered |
| setmetamode | Y |  |  | covered |
| setpalette |  |  |  | missing |
| setpci | Y |  |  | covered |
| setpgid | Y |  |  | covered |
| setpriv | Y |  |  | covered |
| setsid | Y |  | Y | covered |
| setterm | Y |  |  | covered |
| setvesablank | Y |  |  | covered |
| setvtrgb | Y |  |  | covered |
| setxkbmap | Y |  |  | covered |
| sexp-conv |  |  |  | missing |
| sfdisk | Y |  |  | covered |
| sftp | Y |  |  | covered |
| sfv-hash | Y |  |  | covered |
| sh |  |  | Y | covered |
| sha1sum | Y |  |  | covered |
| sha224sum | Y |  |  | covered |
| sha256sum | Y |  |  | covered |
| sha384sum | Y |  |  | covered |
| sha512sum | Y |  |  | covered |
| shapeclustering |  |  |  | missing |
| shlibsign |  |  |  | missing |
| showconsolefont | Y |  |  | covered |
| showdb |  |  |  | missing |
| showjournal |  |  |  | missing |
| showkey | Y |  |  | covered |
| showshm |  |  |  | missing |
| showstat4 |  |  |  | missing |
| showwal |  |  |  | missing |
| shred | Y |  |  | covered |
| shtab |  |  |  | missing |
| shuf | Y |  |  | covered |
| shutdown | Y |  | Y | covered |
| signon-ui |  |  |  | missing |
| signond |  |  |  | missing |
| signonpluginprocess |  |  |  | missing |
| signtool | Y |  |  | covered |
| signver | Y |  |  | covered |
| sim_client |  |  |  | missing |
| sim_server |  |  |  | missing |
| simple_dcraw |  |  |  | missing |
| sixel2png | Y |  |  | covered |
| size | Y |  | Y | covered |
| skdump |  |  |  | missing |
| sktest |  |  |  | missing |
| slabtop | Y |  |  | covered |
| sleep | Y |  | Y | covered |
| sln |  |  |  | missing |
| slsh | Y |  |  | covered |
| slurp | Y |  | Y | covered |
| smb2-quota | Y |  |  | covered |
| smbcacls | Y |  |  | covered |
| smbclient | Y |  |  | covered |
| smbcquotas | Y |  |  | covered |
| smbget | Y |  |  | covered |
| smbinfo | Y |  |  | covered |
| smbspool | Y |  |  | covered |
| smbtar | Y |  |  | covered |
| smbtree | Y |  |  | covered |
| sminfo | Y |  |  | covered |
| smpdump | Y |  |  | covered |
| smpquery | Y |  |  | covered |
| snapper | Y |  |  | covered |
| snapper-sync | Y |  |  | covered |
| snapperd | Y |  |  | covered |
| snbk | Y |  |  | covered |
| sndfile-cmp | Y |  |  | covered |
| sndfile-concat | Y |  |  | covered |
| sndfile-convert | Y |  |  | covered |
| sndfile-deinterleave | Y |  |  | covered |
| sndfile-info | Y |  |  | covered |
| sndfile-interleave | Y |  |  | covered |
| sndfile-metadata-get | Y |  |  | covered |
| sndfile-metadata-set | Y |  |  | covered |
| sndfile-play | Y |  |  | covered |
| sndfile-salvage | Y |  |  | covered |
| sndioctl | Y |  |  | covered |
| sndiod | Y |  |  | covered |
| socat | Y |  |  | covered |
| socat-broker.sh |  |  |  | missing |
| socat-chain.sh |  |  |  | missing |
| socat-mux.sh |  |  |  | missing |
| socat1 | Y |  |  | covered |
| soelim | Y |  |  | covered |
| soffice |  |  |  | missing |
| solid-hardware6 |  |  |  | missing |
| sord_validate | Y |  |  | covered |
| sordi | Y |  |  | covered |
| sort | Y |  | Y | covered |
| sotruss |  |  |  | missing |
| source-highlight | Y | Y |  | covered |
| source-highlight-esc.sh |  |  |  | missing |
| source-highlight-settings | Y |  |  | covered |
| spa-acp-tool | Y |  |  | covered |
| spa-inspect | Y |  |  | covered |
| spa-json-dump | Y |  |  | covered |
| spa-monitor | Y |  |  | covered |
| spa-resample | Y |  |  | covered |
| spawn_console |  |  |  | missing |
| spawn_login |  |  |  | missing |
| spdxtool | Y |  |  | covered |
| speaker-test | Y |  |  | covered |
| speexdec | Y |  |  | covered |
| speexenc | Y |  |  | covered |
| spellout |  |  |  | missing |
| spirv-as |  |  |  | missing |
| spirv-cfg |  |  |  | missing |
| spirv-diff |  |  |  | missing |
| spirv-dis |  |  |  | missing |
| spirv-lesspipe.sh |  |  |  | missing |
| spirv-link |  |  |  | missing |
| spirv-lint |  |  |  | missing |
| spirv-objdump |  |  |  | missing |
| spirv-opt |  |  |  | missing |
| spirv-reduce |  |  |  | missing |
| spirv-val |  |  |  | missing |
| spit | Y |  |  | covered |
| split | Y |  |  | covered |
| split-file |  |  |  | missing |
| sprof |  |  |  | missing |
| sqldiff |  |  |  | missing |
| sqlhist | Y |  |  | covered |
| sqlite3 | Y |  |  | covered |
| sqlite3_expert |  |  |  | missing |
| sqlite3_rsync |  |  |  | missing |
| src-hilite-lesspipe.sh |  |  |  | missing |
| srp_daemon | Y |  |  | covered |
| srp_daemon.sh |  |  |  | missing |
| srt-ffplay |  |  |  | missing |
| srt-file-transmit |  |  |  | missing |
| srt-live-transmit |  |  |  | missing |
| srt-tunnel |  |  |  | missing |
| ss | Y |  |  | covered |
| sserver | Y |  |  | covered |
| ssh | Y |  | Y | covered |
| ssh-add | Y |  |  | covered |
| ssh-agent | Y |  |  | covered |
| ssh-copy-id | Y |  |  | covered |
| ssh-keygen | Y |  |  | covered |
| ssh-keyscan | Y |  |  | covered |
| sshd | Y |  | Y | covered |
| ssltap | Y |  |  | covered |
| starship |  |  |  | missing |
| start-hyprland |  |  |  | missing |
| stat | Y |  | Y | covered |
| stdbuf | Y |  |  | covered |
| storagectl | Y |  |  | covered |
| stream | Y |  | Y | covered |
| strings | Y |  | Y | covered |
| strip | Y |  |  | covered |
| stty | Y |  | Y | covered |
| stunbdc |  |  |  | missing |
| stund |  |  |  | missing |
| su | Y |  |  | covered |
| substrings.pl |  |  |  | missing |
| sudo | Y |  | Y | covered |
| sudo_logsrvd | Y |  |  | covered |
| sudo_sendlog | Y |  |  | covered |
| sudoedit | Y |  |  | covered |
| sudoreplay | Y |  |  | covered |
| sulogin | Y |  |  | covered |
| sum | Y |  |  | covered |
| sushi |  |  |  | missing |
| swaplabel | Y |  |  | covered |
| swapoff | Y |  |  | covered |
| swapon | Y |  |  | covered |
| sway | Y |  | Y | covered |
| swaybar |  |  |  | missing |
| swaybg | Y |  |  | covered |
| swaymsg | Y |  |  | covered |
| swaynag | Y |  |  | covered |
| switch_root | Y |  |  | covered |
| symkeyutil |  |  |  | missing |
| sync | Y |  | Y | covered |
| syntax_suggest |  |  |  | missing |
| syscse |  |  |  | missing |
| sysctl | Y |  | Y | covered |
| system-config-printer | Y |  |  | covered |
| system-config-printer-applet | Y |  |  | covered |
| systemctl | Y |  | Y | covered |
| systemd-ac-power | Y |  |  | covered |
| systemd-analyze | Y |  |  | covered |
| systemd-ask-password | Y |  |  | covered |
| systemd-cat | Y |  |  | covered |
| systemd-cgls | Y |  |  | covered |
| systemd-cgtop | Y |  |  | covered |
| systemd-confext | Y |  |  | covered |
| systemd-creds | Y |  |  | covered |
| systemd-cryptenroll | Y |  |  | covered |
| systemd-cryptsetup | Y |  |  | covered |
| systemd-delta | Y |  |  | covered |
| systemd-detect-virt | Y |  |  | covered |
| systemd-dissect | Y |  |  | covered |
| systemd-escape | Y |  |  | covered |
| systemd-firstboot | Y |  |  | covered |
| systemd-home-fallback-shell |  |  |  | missing |
| systemd-hwdb | Y |  |  | covered |
| systemd-id128 | Y |  |  | covered |
| systemd-inhibit | Y |  |  | covered |
| systemd-machine-id-setup | Y |  |  | covered |
| systemd-mount | Y |  |  | covered |
| systemd-mstack | Y |  |  | covered |
| systemd-mute-console | Y |  |  | covered |
| systemd-notify | Y |  |  | covered |
| systemd-nspawn | Y |  |  | covered |
| systemd-path | Y |  |  | covered |
| systemd-pty-forward | Y |  |  | covered |
| systemd-repart | Y |  |  | covered |
| systemd-resolve |  |  |  | missing |
| systemd-run | Y |  |  | covered |
| systemd-socket-activate | Y |  |  | covered |
| systemd-stdio-bridge | Y |  |  | covered |
| systemd-sysext | Y |  |  | covered |
| systemd-sysinstall | Y |  |  | covered |
| systemd-sysusers | Y |  |  | covered |
| systemd-tmpfiles | Y |  |  | covered |
| systemd-tty-ask-password-agent | Y |  |  | covered |
| systemd-umount | Y |  |  | covered |
| systemd-vmspawn | Y |  |  | covered |
| systemd-vpick | Y |  |  | covered |
| tabs | Y |  |  | covered |
| tac | Y |  |  | covered |
| taglib-config |  |  |  | missing |
| tail | Y |  | Y | covered |
| talk | Y |  |  | covered |
| talkd | Y |  |  | covered |
| tam-native-defaults |  |  |  | missing |
| tam-session |  |  | Y | covered |
| tam-theme-set-browser-policy |  |  |  | missing |
| tar | Y | Y |  | covered |
| taskset | Y |  |  | covered |
| tbl | Y |  |  | covered |
| tc | Y |  |  | covered |
| tcat |  |  |  | missing |
| tdbbackup | Y |  |  | covered |
| tdbdump | Y |  |  | covered |
| tdbrestore | Y |  |  | covered |
| tdbtool | Y |  |  | covered |
| teamd | Y |  |  | covered |
| teamdctl | Y |  |  | covered |
| teamnl | Y |  |  | covered |
| tee | Y |  | Y | covered |
| telnet | Y |  |  | covered |
| telnetd | Y |  |  | covered |
| tensaku | Y |  |  | covered |
| tensaku-edit |  |  |  | missing |
| tesseract |  |  |  | missing |
| test | Y |  | Y | covered |
| test-runner |  |  |  | missing |
| testcontroller |  |  |  | missing |
| testlibraw | Y |  |  | covered |
| testpkg | Y |  |  | covered |
| texi2any | Y |  |  | covered |
| texi2dvi | Y |  |  | covered |
| texi2pdf | Y |  |  | covered |
| texindex | Y |  |  | covered |
| text2image |  |  |  | missing |
| tfmtodit | Y |  |  | covered |
| tic | Y |  |  | covered |
| tiff2bw |  |  |  | missing |
| tiff2pdf |  |  |  | missing |
| tiff2ps |  |  |  | missing |
| tiff2rgba |  |  |  | missing |
| tiffcmp |  |  |  | missing |
| tiffcp |  |  |  | missing |
| tiffcrop |  |  |  | missing |
| tiffdither |  |  |  | missing |
| tiffdump |  |  |  | missing |
| tiffgt |  |  |  | missing |
| tiffinfo |  |  |  | missing |
| tiffmedian |  |  |  | missing |
| tiffset |  |  |  | missing |
| tiffsplit |  |  |  | missing |
| tificc | Y |  |  | covered |
| tiger-hash | Y |  |  | covered |
| timedatectl | Y |  |  | covered |
| timelineeditor |  |  |  | missing |
| timeout | Y |  | Y | covered |
| tinysparql |  |  |  | missing |
| tipc | Y |  |  | covered |
| tjbench |  |  |  | missing |
| tldr | Y |  |  | covered |
| tload | Y |  |  | covered |
| tmux | Y |  | Y | covered |
| toast | Y |  |  | covered |
| toe | Y |  |  | covered |
| top | Y |  | Y | covered |
| touch | Y |  |  | covered |
| touchpad-edge-detector | Y |  |  | covered |
| tput | Y |  |  | covered |
| tqdm |  |  |  | missing |
| tr | Y |  | Y | covered |
| tracepath | Y |  |  | covered |
| transicc | Y |  |  | covered |
| tree-sitter |  |  |  | missing |
| trietool | Y |  |  | covered |
| trietool-0.2 | Y |  |  | covered |
| troff | Y |  |  | covered |
| true | Y |  | Y | covered |
| truncate | Y |  |  | covered |
| trust | Y |  |  | covered |
| try |  |  | Y | covered |
| ts_calibrate | Y |  |  | covered |
| ts_conf | Y |  |  | covered |
| ts_finddev | Y |  |  | covered |
| ts_harvest | Y |  |  | covered |
| ts_print | Y |  |  | covered |
| ts_print_mt | Y |  |  | covered |
| ts_print_raw | Y |  |  | covered |
| ts_test | Y |  |  | covered |
| ts_test_mt | Y |  |  | covered |
| ts_uinput | Y |  |  | covered |
| ts_verify | Y |  |  | covered |
| tset | Y |  |  | covered |
| tsort | Y |  |  | covered |
| ttfx |  |  |  | missing |
| tth-hash | Y |  |  | covered |
| tty | Y |  | Y | covered |
| tune.exfat | Y |  |  | covered |
| tune2fs | Y |  |  | covered |
| tunelp | Y |  |  | covered |
| tzselect | Y |  |  | covered |
| tzupdate |  |  |  | missing |
| uchardet | Y |  |  | covered |
| uclampset | Y |  |  | covered |
| ucmatose | Y |  |  | covered |
| uconv | Y |  |  | covered |
| udaddy | Y |  |  | covered |
| udevadm | Y |  |  | covered |
| udiskie | Y |  |  | covered |
| udiskie-info |  |  |  | missing |
| udiskie-mount |  |  |  | missing |
| udiskie-umount |  |  |  | missing |
| udisksctl | Y |  |  | covered |
| udpong | Y |  |  | covered |
| ufw | Y |  |  | covered |
| ufw-docker |  |  |  | missing |
| ul | Y |  | Y | covered |
| umount | Y |  | Y | covered |
| umount.udisks2 | Y |  |  | covered |
| uname | Y |  |  | covered |
| uname26 | Y |  |  | covered |
| uncompress |  |  |  | missing |
| unexpand | Y |  |  | covered |
| unicharset_extractor |  |  |  | missing |
| unicode_start | Y |  |  | covered |
| unicode_stop | Y |  |  | covered |
| uniq | Y |  | Y | covered |
| unix_chkpwd | Y |  |  | covered |
| unixcmd | Y |  |  | covered |
| unlink | Y |  | Y | covered |
| unlz4 | Y |  |  | covered |
| unlzma | Y |  |  | covered |
| unmunch |  |  |  | missing |
| unopkg | Y |  |  | covered |
| unprocessed_raw |  |  |  | missing |
| unshare | Y |  |  | covered |
| unterm |  |  |  | missing |
| untoast |  |  |  | missing |
| unxz | Y |  |  | covered |
| unzip | Y |  |  | covered |
| unzipsfx | Y |  |  | covered |
| unzstd | Y |  |  | covered |
| update-ca-trust | Y |  |  | covered |
| update-desktop-database | Y |  |  | covered |
| update-mime-database | Y |  |  | covered |
| update-pciids | Y |  |  | covered |
| update_compids.sh |  |  |  | missing |
| updatectl | Y |  |  | covered |
| updatedb | Y |  |  | covered |
| updpkgsums | Y |  |  | covered |
| upower | Y |  | Y | covered |
| uptime | Y |  | Y | covered |
| usage |  |  | Y | covered |
| useradd | Y |  |  | covered |
| userdbctl | Y |  |  | covered |
| userdel | Y |  |  | covered |
| usermod | Y |  |  | covered |
| users | Y |  | Y | covered |
| utmpdump | Y |  |  | covered |
| uuclient |  |  |  | missing |
| uuctl | Y |  |  | covered |
| uuidd | Y |  |  | covered |
| uuidgen | Y |  |  | covered |
| uuidparse | Y |  |  | covered |
| uuserver |  |  |  | missing |
| uwsm | Y |  |  | covered |
| uwsm-app | Y |  |  | covered |
| uwsm-terminal |  |  |  | missing |
| uwsm-terminal-scope |  |  |  | missing |
| uwsm-terminal-service |  |  |  | missing |
| v4l2-compliance | Y |  |  | covered |
| v4l2-ctl | Y |  |  | covered |
| v4l2-dbg |  |  |  | missing |
| v4l2-sysfs-path |  |  |  | missing |
| v4l2-tracer | Y |  |  | covered |
| vacuumdb | Y |  |  | covered |
| vacuumlo | Y |  |  | covered |
| vapoursynth |  |  |  | missing |
| varlinkctl | Y |  |  | covered |
| vbltest |  |  |  | missing |
| vde_autolink | Y |  |  | covered |
| vde_cryptcab | Y |  |  | covered |
| vde_over_ns | Y |  |  | covered |
| vde_pcapplug | Y |  |  | covered |
| vde_plug | Y |  |  | covered |
| vde_plug2tap | Y |  |  | covered |
| vde_router | Y |  |  | covered |
| vde_switch | Y |  |  | covered |
| vdecmd | Y |  |  | covered |
| vdeterm | Y |  |  | covered |
| vdir | Y |  |  | covered |
| vdpa | Y |  |  | covered |
| vendstat | Y |  |  | covered |
| vercmp | Y |  |  | covered |
| verify-uselistorder |  |  |  | missing |
| veritysetup | Y |  |  | covered |
| vfat-resize |  |  |  | missing |
| vigr | Y |  |  | covered |
| vips | Y |  |  | covered |
| vipsedit | Y |  |  | covered |
| vipsheader | Y |  |  | covered |
| vipsthumbnail | Y |  |  | covered |
| vipw | Y |  |  | covered |
| virtiofsd |  |  |  | missing |
| visudo | Y |  |  | covered |
| vlock | Y |  |  | covered |
| vmaf |  |  |  | missing |
| vmstat | Y |  | Y | covered |
| volume_key | Y |  |  | covered |
| voxtype |  |  |  | missing |
| voxtype-audio-bridge |  |  |  | missing |
| voxtype-configure-launcher |  |  |  | missing |
| voxtype-osd |  |  |  | missing |
| vpxdec |  |  |  | missing |
| vpxenc |  |  |  | missing |
| vsd2raw |  |  |  | missing |
| vsd2text |  |  |  | missing |
| vsd2xhtml |  |  |  | missing |
| vspipe |  |  |  | missing |
| vss2raw |  |  |  | missing |
| vss2text |  |  |  | missing |
| vss2xhtml |  |  |  | missing |
| vterm-ctrl |  |  |  | missing |
| vterm-dump |  |  |  | missing |
| w | Y |  | Y | covered |
| w3m | Y |  |  | covered |
| w3mman | Y |  |  | covered |
| wait-tray |  |  |  | missing |
| waitpid | Y |  |  | covered |
| wall | Y |  |  | covered |
| wasmdeployqt6 |  |  |  | missing |
| watch | Y |  |  | covered |
| watchgnupg | Y |  |  | covered |
| wayland-scanner |  |  |  | missing |
| wc | Y |  | Y | covered |
| wcurl | Y |  |  | covered |
| wdctl | Y |  |  | covered |
| wev | Y |  |  | covered |
| whatis | Y |  | Y | covered |
| whereis | Y |  |  | covered |
| which | Y | Y | Y | covered |
| whiptail | Y |  |  | covered |
| whirlpool-hash | Y |  |  | covered |
| who | Y |  | Y | covered |
| whoami | Y |  |  | covered |
| whois | Y |  |  | covered |
| wipefs | Y |  |  | covered |
| wirefilter | Y |  |  | covered |
| wireplumber | Y |  |  | covered |
| wks2csv |  |  |  | missing |
| wks2raw |  |  |  | missing |
| wks2text |  |  |  | missing |
| wl-copy | Y |  |  | covered |
| wl-paste | Y |  |  | covered |
| woff2_compress |  |  |  | missing |
| woff2_decompress |  |  |  | missing |
| woff2_info |  |  |  | missing |
| wordforms |  |  |  | missing |
| wordlist2dawg |  |  |  | missing |
| wordlist2hunspell |  |  |  | missing |
| wpa_cli | Y |  |  | covered |
| wpa_passphrase | Y |  |  | covered |
| wpa_supplicant | Y |  |  | covered |
| wpctl | Y |  |  | covered |
| wpd2html |  |  |  | missing |
| wpd2raw |  |  |  | missing |
| wpd2text |  |  |  | missing |
| wpexec | Y |  |  | covered |
| wps2html |  |  |  | missing |
| wps2raw |  |  |  | missing |
| wps2text |  |  |  | missing |
| write | Y |  | Y | covered |
| wrjpgcom | Y |  |  | covered |
| wtype | Y |  |  | covered |
| x264 |  |  |  | missing |
| x265 |  |  |  | missing |
| x86_64 | Y |  | Y | covered |
| x86_64-linux-gnu-c++ |  |  |  | missing |
| x86_64-linux-gnu-g++ |  |  |  | missing |
| x86_64-linux-gnu-gcc |  |  |  | missing |
| x86_64-linux-gnu-gcc-ar |  |  |  | missing |
| x86_64-linux-gnu-gcc-nm |  |  |  | missing |
| x86_64-linux-gnu-gcc-ranlib |  |  |  | missing |
| x86_64-pc-linux-gnu-c++ |  |  |  | missing |
| x86_64-pc-linux-gnu-g++ |  |  |  | missing |
| x86_64-pc-linux-gnu-gcc |  |  |  | missing |
| x86_64-pc-linux-gnu-gcc-16 |  |  |  | missing |
| x86_64-pc-linux-gnu-gcc-ar |  |  |  | missing |
| x86_64-pc-linux-gnu-gcc-nm |  |  |  | missing |
| x86_64-pc-linux-gnu-gcc-ranlib |  |  |  | missing |
| x86_64-pc-linux-gnu-pkg-config |  |  |  | missing |
| xargs | Y |  | Y | covered |
| xauth | Y |  |  | covered |
| xb-tool | Y |  |  | covered |
| xdg-dbus-proxy | Y |  |  | covered |
| xdg-desktop-icon | Y |  |  | covered |
| xdg-desktop-menu | Y |  |  | covered |
| xdg-email | Y |  |  | covered |
| xdg-icon-resource | Y |  |  | covered |
| xdg-mime | Y |  |  | covered |
| xdg-open | Y |  |  | covered |
| xdg-screensaver | Y |  |  | covered |
| xdg-settings | Y |  |  | covered |
| xdg-terminal-exec | Y |  |  | covered |
| xdg-user-dir | Y |  |  | covered |
| xdg-user-dirs-gtk-update |  |  |  | missing |
| xdg-user-dirs-update | Y |  |  | covered |
| xgettext | Y |  |  | covered |
| xkbcli | Y |  |  | covered |
| xkbcomp | Y |  |  | covered |
| xml2-config | Y |  |  | covered |
| xmlcatalog | Y |  |  | covered |
| xmllint | Y |  |  | covered |
| xmlsec1 | Y |  |  | covered |
| xmlsec1-config | Y |  |  | covered |
| xmlwf |  |  |  | missing |
| xournalpp | Y |  |  | covered |
| xournalpp-thumbnailer | Y |  |  | covered |
| xournalpp-wrapper |  |  |  | missing |
| xprop | Y |  |  | covered |
| xpstojpeg | Y |  |  | covered |
| xpstopdf | Y |  |  | covered |
| xpstopng | Y |  |  | covered |
| xpstops | Y |  |  | covered |
| xpstosvg | Y |  |  | covered |
| xslt-config |  |  |  | missing |
| xsltproc | Y |  |  | covered |
| xtables-legacy-multi |  |  |  | missing |
| xtables-monitor | Y |  |  | covered |
| xtables-nft-multi |  |  |  | missing |
| xtotroff | Y |  |  | covered |
| xtrace |  |  |  | missing |
| xtractprotos |  |  |  | missing |
| xxh128sum | Y |  |  | covered |
| xxh32sum | Y |  |  | covered |
| xxh3sum | Y |  |  | covered |
| xxh64sum | Y |  |  | covered |
| xxhsum | Y |  |  | covered |
| xz | Y |  |  | covered |
| xzcat | Y |  |  | covered |
| xzcmp | Y |  |  | covered |
| xzdec | Y |  |  | covered |
| xzdiff | Y |  |  | covered |
| xzegrep | Y |  |  | covered |
| xzfgrep | Y |  |  | covered |
| xzgrep | Y |  |  | covered |
| xzless | Y |  |  | covered |
| xzmore | Y |  |  | covered |
| yacc | Y |  |  | covered |
| yaml-bench |  |  |  | missing |
| yaml2obj |  |  |  | missing |
| yat2m |  |  |  | missing |
| yay | Y |  |  | covered |
| yes | Y |  |  | covered |
| yt-dlp | Y |  |  | covered |
| yuvconvert |  |  |  | missing |
| zbarcam | Y |  |  | covered |
| zbarcam-gtk |  |  |  | missing |
| zbarcam-qt |  |  |  | missing |
| zbarimg | Y |  |  | covered |
| zcat | Y |  |  | covered |
| zcmp | Y |  |  | covered |
| zdiff | Y |  |  | covered |
| zdump | Y |  |  | covered |
| zegrep |  |  |  | missing |
| zfgrep |  |  |  | missing |
| zforce | Y |  |  | covered |
| zgrep | Y |  |  | covered |
| zic | Y |  |  | covered |
| zint | Y |  |  | covered |
| zip | Y |  |  | covered |
| zipcloak | Y |  |  | covered |
| zipcmp | Y |  |  | covered |
| zipgrep | Y |  |  | covered |
| zipinfo | Y |  |  | covered |
| zipmerge | Y |  |  | covered |
| zipnote | Y |  |  | covered |
| zipsplit | Y |  |  | covered |
| ziptool | Y |  |  | covered |
| zless | Y |  |  | covered |
| zlib-flate | Y |  |  | covered |
| zmf2raw |  |  |  | missing |
| zmf2svg |  |  |  | missing |
| zmore | Y |  |  | covered |
| znew | Y |  |  | covered |
| zoxide | Y |  |  | covered |
| zramctl | Y |  | Y | covered |
| zsoelim | Y |  |  | covered |
| zstd | Y |  | Y | covered |
| zstdcat | Y |  |  | covered |
| zstdgrep | Y |  |  | covered |
| zstdless | Y |  |  | covered |
| zstdmt |  |  |  | missing |