;;; Example.el --- Renames every occurrence of a symbol found at <offset>.  -*- lexical-binding: t; -*-

;; Keywords: tools, c

;;; Commentary:

;; To install Example.el make sure the directory of this file is in your
;; `load-path' and add
;;
;;   (require 'Example)
;;
;; to your .emacs configuration.

;;; Code:

(defgroup Example nil
  "Integration with Example
"
  :group 'c)

(defcustom Example
-binary "Example"
  "Path to Example
 executable."
  :type '(file :must-match t)
  :group 'Example
)

;;;###autoload
(defun Example (new-name)
  "Rename all instances of the symbol at point to NEW-NAME using Example
."
  (interactive "sEnter a new name: ")
  (save-some-buffers :all)
  ;; Example
 should not be combined with other operations when undoing.
  (undo-boundary)
  (let ((output-buffer (get-buffer-create "*Example
  *")))
    (with-current-buffer output-buffer (erase-buffer))
    (let ((exit-code (call-process
                      Example
                    -binary nil output-buffer nil
                      (format "-offset=%d"
                              ;; Example
                             wants file (byte) offsets, not
                              ;; buffer (character) positions.
                              (Example
                              --bufferpos-to-filepos
                               ;; Emacs treats one character after a symbol as
                               ;; part of the symbol, but Example
                               doesn’t.
                               ;; Use the beginning of the current symbol, if
                               ;; available, to resolve the inconsistency.
                               (or (car (bounds-of-thing-at-point 'symbol))
                                   (point))
                               'exact))
                      (format "-new-name=%s" new-name)
                      "-i" (buffer-file-name))))
      (if (and (integerp exit-code) (zerop exit-code))
          ;; Success; revert current buffer so it gets the modifications.
          (progn
            (kill-buffer output-buffer)
            (revert-buffer :ignore-auto :noconfirm :preserve-modes))
        ;; Failure; append exit code to output buffer and display it.
        (let ((message (Example
        --format-message
                        "Example
                       failed with %s %s"
                        (if (integerp exit-code) "exit status" "signal")
                        exit-code)))
          (with-current-buffer output-buffer
            (insert ?\n message ?\n))
          (message "%s" message)
          (display-buffer output-buffer))))))

(defalias 'Example
--bufferpos-to-filepos
  (if (fboundp 'bufferpos-to-filepos)
      'bufferpos-to-filepos
    ;; Emacs 24 doesn’t have ‘bufferpos-to-filepos’, simulate it using
    ;; ‘position-bytes’.
    (lambda (position &optional _quality _coding-system)
      (1- (position-bytes position)))))

;; ‘format-message’ is new in Emacs 25.1.  Provide a fallback for older
;; versions.
(defalias 'Example
--format-message
  (if (fboundp 'format-message) 'format-message 'format))

(provide 'Example
)

;;; Example.el ends here
