KLIN ?= klin

.PHONY: emit clean

emit: out/smoke.c

out/smoke.c: smoke.kl $(wildcard ../../esp_usb/*.kl)
	@mkdir -p out
	@case "$(KLIN)" in \
	  *.dart) dart run "$(KLIN)" --emit-c smoke.kl ;; \
	  *) "$(KLIN)" --emit-c smoke.kl ;; \
	esac

clean:
	rm -rf out
