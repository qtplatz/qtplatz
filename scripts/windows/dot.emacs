(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(ansi-color-faces-vector
   [default default default italic underline success warning error])
 '(ansi-color-names-vector
   ["#242424" "#e5786d" "#95e454" "#cae682" "#8ac6f2" "#333366" "#ccaa8f" "#f6f3e8"])
 '(custom-enabled-themes (quote (tsdh-dark)))
 '(inhibit-startup-screen t)
 '(package-selected-packages
   (quote
    (modern-cpp-font-lock egg git gnuplot-mode dts-mode cuda-mode mozc-im langtool flycheck-rtags company-rtags ac-mozc))))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(default ((t (:family "Noto Mono" :foundry "monotype" :slant normal :weight normal :height 90 :width normal)))))
;;;;;;;;;;;;;;;;;;;
(require 'package)
(setq package-archives '(("gnu" . "https://elpa.gnu.org/packages/")
			 ("marmalade" . "https://marmalade-repo.org/packages/")
			 ("melpa" . "https://melpa.org/packages/")))

(package-initialize)

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

(add-hook 'sh-mode-hook
          '(lambda ()
             (setq tab-width 4)
             (setq indent-tabs-mode t)))

(setq split-height-threshold 0)
(setq split-width-threshold 0)
;;
;;(require 'rtags)
;;(require 'company)
;;(require 'company-rtags)
;;(setq rtags-autostart-diagnostics t)
;;(rtags-diagnostics)
;;(setq rtags-completions-enabled t)
;;(push 'company-rtags company-backends)
;;(global-company-mode)
;;(rtags-enable-standard-keybindings)
;;(define-key c-mode-base-map (kbd "<C-tab>") (function company-complete))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(modern-c++-font-lock-global-mode t)

