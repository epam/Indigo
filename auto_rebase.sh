#!/bin/bash
# Автоматическое разрешение конфликтов при rebase

while git status | grep -q "Rebase in progress"; do
    # Разрешаем все конфликты нашими версиями
    git status --short | grep '^UU\|^DU\|^UD\|^AA' | awk '{print $2}' | while read file; do
        echo "Resolving conflict in $file with --ours"
        git checkout --ours "$file" 2>/dev/null || true
    done

    # Добавляем все изменения
    git add .

    # Пробуем продолжить
    git rebase --continue 2>&1 | tee /tmp/rebase_output.txt

    # Если получили "No changes", пропускаем коммит
    if grep -q "No changes" /tmp/rebase_output.txt; then
        echo "Skipping empty commit..."
        git rebase --skip
    fi

    # Если ошибка, но не из-за конфликтов - выходим
    if [ $? -ne 0 ] && ! git status | grep -q "Unmerged paths"; then
        echo "Rebase finished or encountered unexpected error"
        break
    fi

    # Проверяем, не завершен ли rebase
    if ! git status | grep -q "Rebase in progress"; then
        echo "Rebase completed successfully!"
        break
    fi
done
