// NOTE: Keep module path unchanged so downstream projects can use `replace`
// without having to rewrite imports (e.g. gorm's sqlite dialect still imports this path).
module github.com/mattn/go-sqlite3

go 1.19

retract (
 [v2.0.0+incompatible, v2.0.6+incompatible] // Accidental; no major changes or features.
)
