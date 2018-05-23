(require 'package)
(package-initialize)
(add-to-list 'package-archives
             '("melpa" . "http://melpa.milkbox.net/packages/") t)

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

(setq split-height-threshold 0)
(setq split-width-threshold 0)

;;;;;
;(add-to-list 'load-path "~/.emacs.d/")
(require 'auto-complete-config)
(add-to-list 'ac-dictionary-directories "~/.emacs.d//ac-dict")
(ac-config-default)

;;;
(set-face-attribute 'default nil :height 100)

;;;;;;;; CEDET config ;;;;;;;;;;;
;; select which submodes we want to activate

;;(global-ede-mode 1)
;;(semantic-mode 1)
;(global-srecode-minor-mode 1)
;;(ede-enable-generic-projects)
;;(require 'semantic/ia)

;;(global-semanticdb-minor-mode 1)
;;(global-semantic-idle-scheduler-mode 1)
;;(global-semantic-idle-completions-mode 1)
;;(global-semantic-idle-summary-mode 1)

;;(global-semantic-stickyfunc-mode 1)
;;;(global-cedet-m3-minor-mode 1)
;;(global-semantic-highlight-func-mode 1)
;;(global-semantic-mru-bookmark-mode 1)
;;(global-cedit-m3-minor-mode 1)

;;(semanticdb-enable-gnu-global-databases 'c-mode t)
;;(semanticdb-enable-gnu-global-databases 'c++-mode t)

;;(defun my-semantic-hook ()
;;  (imenu-add-to-menubar "TAGS"))
;;(add-hook 'semantic-init-hooks 'my-semantic-hook)

;;(setq boost-base-dir "C:/Boost/include/boost-1_54/boost")
;;(setq qt5-base-dir "C:/Qt/Qt5.1.1/5.1.1/msvc2012/include")

;;(semantic-add-system-include boost-base-dir 'c++-mode)
;;(semantic-add-system-include qt5-base-dir 'c++-mode)
;;(semantic-add-system-include (concat qt5-base-dir "/QtWidget" 'c++-mode)
;;(semantic-add-system-include (concat qt5-base-dir "/QtGui" 'c++-mode)
;;(semantic-add-system-include (concat qt5-base-dir "/QtCore" 'c++-mode)

;(define-key 'c++-mode "." 'semantic-complete-self-insert)

;semantic-speedbar-analysis

(add-to-list 'default-frame-alist '(background-color . "#f0ffff"))
(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(inhibit-startup-screen t))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(default ((t (:family "Consolas" :foundry "outline" :slant normal :weight normal :height 98 :width normal)))))

;; wl
(autoload 'wl "wl" "Wanderlust" t)
(autoload 'wl-draft "wl" "Write draft with Wanderlust." t)

;;
(setq browse-url-browser-function 'browse-url-generic)
(setq browse-url-generic-program 
      (if (file-exists-p "/usr/bin/chromium")
          "/usr/bin/chromium" "/usr/bin/google-chrome"))

