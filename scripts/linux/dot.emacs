(require 'package)
(setq package-archives '(("melpa" . "https://melpa.org/packages/")
                         ("org" . "https://orgmode.org/elpa/")) )
(package-initialize)
(require 'use-package)
;;;;;;;;;;
;;;;;;;;;;
(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(ansi-color-names-vector
   ["#242424" "#e5786d" "#95e454" "#cae682" "#8ac6f2" "#333366" "#ccaa8f" "#f6f3e8"])
 '(custom-enabled-themes (quote (tsdh-dark)))
 '(package-selected-packages
   (quote
    (use-package cmake-font-lock cmake-ide cmake-mode company vhdl-tools company-irony-c-headers company-irony egg git gnuplot-mode dts-mode cuda-mode mozc-im langtool flycheck-rtags company-rtags ac-mozc))))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(default ((t (:family "DejaVu Sans Mono" :foundry "unknown" :slant normal :weight normal :height 90 :width normal)))))
;;;;;;;;;;;;;;;;;;;

(add-hook 'c-mode-common-hook
          '(lambda ()
             (c-set-style "stroustrup")
             (setq c-basic-offset 4)
             (setq tab-width 4)
             (setq indent-tabs-mode nil)))

(add-hook 'c++-mode-hook
	  '(lambda ()
	     (c-set-style "stroustrup")
             (setq c-basic-offset 4)
             (setq tab-width 4)
	     (setq indent-tabs-mode nil)))

(add-hook 'c++-mode-hook 'rtags-start-process-unless-running)

(add-hook 'sh-mode-hook
          '(lambda ()
             (setq tab-width 4)
             (setq indent-tabs-mode t)))

(setq split-height-threshold 0)
(setq split-width-threshold 0)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

 (add-hook 'before-save-hook 'delete-trailing-whitespace)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;(when window-system
;  (create-fontset-from-ascii-font "Monaco:pixelsize=11" nil "myfontset")
;  (set-fontset-font "fontset-myfontset"
;                    'japanese-jisx0208 ;'unicode
;                    (font-spec :family "Osaka" :size 11)
;                    nil
;                    'append)
;  (add-to-list 'default-frame-alist '(font . "fontset-myfontset")))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(use-package rtags
  :config
  (setq rtags-autostart-diagnostics t)
  (rtags-diagnostics)
  (setq rtags-completions-enabled t)
  (rtags-enable-standard-keybindings))

(use-package company)
(use-package company-rtags)
(push 'company-rtags company-backends)
(global-company-mode)
(define-key c-mode-base-map (kbd "<C-tab>") (function company-complete))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;(require 'git)
;;(require 'egg)
(eval-after-load "irony"
  '(progn
     (custom-set-variables '(irony-additional-clang-options '("-std=c++11")))
     (add-to-list 'company-backends 'company-irony)
     (add-hook 'irony-mode-hook 'irony-cdb-autosetup-compile-options)
     (add-hook 'c-mode-common-hook 'irony-mode)))
;;
(setq-default indent-tabs-mode nil)
;;
;;;;;
(setq browse-url-browser-function 'browse-url-generic)
(setq browse-url-generic-program
      (if (file-exists-p "/usr/bin/chromium")
          "/usr/bin/chromium" "/usr/bin/google-chrome"))
;;;;;;;;;;;;;;;;;
(use-package mozc
  :ensure t
  :config
  (set-language-environment "Japanese")
  (setq default-input-method "japanese-mozc")
  (prefer-coding-system 'utf-8))

;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;; LanguageTool ;;;;;;
(use-package langtool
  :ensure t
  :config
  (setq langtool-language-tool-jar "/opt/LanguageTool-4.8/languagetool-commandline.jar")
  (setq langtool-default-language "en-US"))

;;;;;;;;;;;;;;;;;;;;;
;;;;; Org mode ;;;;;;
(use-package org
  :ensure t
  :config
  (define-key global-map "\C-cl" 'org-store-link)
  (define-key global-map "\C-ca" 'org-agenda)
  (setq org-log-done t))

(add-hook 'org-mode-hook
          '(lambda ()
             (local-set-key "\M-\C-g" 'org-plot/gnuplot)))

(setq org-agenda-files (list "~/org/todo.org"))
